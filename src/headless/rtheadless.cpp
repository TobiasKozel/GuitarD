#include <stdio.h>
#include <fstream>
#include <string>
#include <iostream>
#include <cstdlib>
#include <cstring>
#include <thread>
#include <map>

#include "./compile_unit/GHeadlessUnit.h" // you can also use the compile_unit version and link against it to speed up things
// #include "./GHeadless.h"
#include "../../thirdparty/rtaudio/RtAudio.h"

#include "../../thirdparty/soundwoofer/soundwooferFile.h"

#include "../types/GRingBuffer.h"

guitard::GuitarDHeadless headless;

#define CHANNEL_COUNT 2
#define MONO_IN // Means it still is a stereo device, but only the left input will be used

guitard::MultiRingBuffer<float, 2> in, out;

std::thread thread;


// Pass-through function.
int callback(void* outputBuffer, void* inputBuffer, unsigned int nBufferFrames, double streamTime, RtAudioStreamStatus status, void*) {
  if (status) {
    printf("\nStream over/underflow detected.\n");
  }

  float* _in = static_cast<float*>(inputBuffer);
  float* _out = static_cast<float*>(outputBuffer);

#ifdef MONO_IN
  in.add(_in, nBufferFrames, 0);
  in.add(_in, nBufferFrames, 1);
#else
  in.add(_in, nBufferFrames, 0);
  in.add(_in + nBufferFrames, nBufferFrames, 1);
#endif

  if (out.inBuffer() >= nBufferFrames) {
    out.get(&_out[0], nBufferFrames, 0);
    out.get(&_out[nBufferFrames], nBufferFrames, 1);
  } else {
    printf("\nUnderrun!\n");
    for (int i = 0; i < nBufferFrames * CHANNEL_COUNT; i++) {
      _out[i] = 0;
    }
  }
  return 0;
}

void audioprobe();

/**
 * Takes a few arguments
 * First one is
 */
int main(int argc, char** argv) {
  audioprobe();

  std::string folder = "../../thirdparty/soundwoofer/dummy_backend/presets/";

  if (argc == 4) {
    folder = argv[3];
  }

  std::vector<soundwoofer::file::FileInfo> presets = soundwoofer::file::scanDir(folder);
  int currentPresetIndex = 0;

  if (presets.empty()) { 
    printf("Preset folder not found!\n");
    return -1;
  }

  RtAudio adac;
  adac.showWarnings( true );
  if (adac.getDeviceCount() < 1) {
    printf("\nNo audio devices found!\n");
    return -1;
  }

  // Set the same number of channels for both input and output.
  unsigned int bufferFrames = 128, sampleRate = 44100;
  RtAudio::StreamParameters iParams, oParams;
  iParams.deviceId = 0;
  iParams.nChannels = CHANNEL_COUNT;
  oParams.deviceId = 0;
  oParams.nChannels = CHANNEL_COUNT;
  RtAudio::StreamOptions streamOptions;
  streamOptions.flags |= RTAUDIO_NONINTERLEAVED;
  streamOptions.flags |= RTAUDIO_MINIMIZE_LATENCY;
  streamOptions.flags |= RTAUDIO_SCHEDULE_REALTIME;

  if (argc > 1) {
    bufferFrames = atoi(argv[1]);
  }
  if (argc > 2) {
    sampleRate = atoi(argv[2]);
  }

  in.setSize(bufferFrames * 4);
  out.setSize(bufferFrames * 4);

  try {
    adac.openStream(&oParams, &iParams, RTAUDIO_FLOAT32, sampleRate, &bufferFrames, &callback, nullptr, &streamOptions);
  } catch (RtAudioError& e) {
    e.printMessage();
    return -2;
  }

  try {
    adac.startStream();
    headless.setConfig(sampleRate, CHANNEL_COUNT, CHANNEL_COUNT);

    thread = std::thread([](){
      while(42) {
        int samples = in.inBuffer();
        if (samples) {
          float* _in[CHANNEL_COUNT], * _out[CHANNEL_COUNT];
          in.get(_in, samples);
          headless.process(_in, _out, samples);
          out.add(_out, samples);
        }
      }
    });

    while (42) {
      std::ifstream file(presets[currentPresetIndex].absolute);
      std::string contents((std::istreambuf_iterator<char>(file)), std::istreambuf_iterator<char>());
      headless.load(contents.c_str());
      printf("Loaded Preset %i ", currentPresetIndex);
      printf("%s\n", presets[currentPresetIndex].name.c_str());
      printf("Press to load the next preset\n");
      getchar();
      currentPresetIndex++;
      if (currentPresetIndex >= presets.size()) {
        currentPresetIndex = 0;
      }
    }
    // Stop the stream.
    thread.join();
    adac.stopStream();
  } catch (RtAudioError& e) {
    e.printMessage();
    if (adac.isStreamOpen()) { adac.closeStream(); }
  }

  return 0;
}


void audioprobe() {
  std::map<int, std::string> apiMap;
  apiMap[RtAudio::MACOSX_CORE] = "OS-X Core Audio";
  apiMap[RtAudio::WINDOWS_ASIO] = "Windows ASIO";
  apiMap[RtAudio::WINDOWS_DS] = "Windows DirectSound";
  apiMap[RtAudio::WINDOWS_WASAPI] = "Windows WASAPI";
  apiMap[RtAudio::UNIX_JACK] = "Jack Client";
  apiMap[RtAudio::LINUX_ALSA] = "Linux ALSA";
  apiMap[RtAudio::LINUX_PULSE] = "Linux PulseAudio";
  apiMap[RtAudio::LINUX_OSS] = "Linux OSS";
  apiMap[RtAudio::RTAUDIO_DUMMY] = "RtAudio Dummy";

  std::vector< RtAudio::Api > apis;
  RtAudio :: getCompiledApi( apis );

  std::cout << "\nRtAudio Version " << RtAudio::getVersion() << std::endl;

  std::cout << "\nCompiled APIs:\n";
  for ( unsigned int i=0; i<apis.size(); i++ )
    std::cout << "  " << apiMap[ apis[i] ] << std::endl;

  RtAudio audio;
  RtAudio::DeviceInfo info;

  std::cout << "\nCurrent API: " << apiMap[ audio.getCurrentApi() ] << std::endl;

  unsigned int devices = audio.getDeviceCount();
  std::cout << "\nFound " << devices << " device(s) ...\n";

  for (unsigned int i=0; i<devices; i++) {
    info = audio.getDeviceInfo(i);

    std::cout << "\nDevice Name = " << info.name << '\n';
    std::cout << "Device ID = " << i << '\n';
    if ( info.probed == false )
      std::cout << "Probe Status = UNsuccessful\n";
    else {
      std::cout << "Probe Status = Successful\n";
      std::cout << "Output Channels = " << info.outputChannels << '\n';
      std::cout << "Input Channels = " << info.inputChannels << '\n';
      std::cout << "Duplex Channels = " << info.duplexChannels << '\n';
      if ( info.isDefaultOutput ) std::cout << "This is the default output device.\n";
      else std::cout << "This is NOT the default output device.\n";
      if ( info.isDefaultInput ) std::cout << "This is the default input device.\n";
      else std::cout << "This is NOT the default input device.\n";
      if ( info.nativeFormats == 0 )
        std::cout << "No natively supported data formats(?)!";
      else {
        std::cout << "Natively supported data formats:\n";
        if ( info.nativeFormats & RTAUDIO_SINT8 )
          std::cout << "  8-bit int\n";
        if ( info.nativeFormats & RTAUDIO_SINT16 )
          std::cout << "  16-bit int\n";
        if ( info.nativeFormats & RTAUDIO_SINT24 )
          std::cout << "  24-bit int\n";
        if ( info.nativeFormats & RTAUDIO_SINT32 )
          std::cout << "  32-bit int\n";
        if ( info.nativeFormats & RTAUDIO_FLOAT32 )
          std::cout << "  32-bit float\n";
        if ( info.nativeFormats & RTAUDIO_FLOAT64 )
          std::cout << "  64-bit float\n";
      }
      if ( info.sampleRates.size() < 1 )
        std::cout << "No supported sample rates found!";
      else {
        std::cout << "Supported sample rates = ";
        for (unsigned int j=0; j<info.sampleRates.size(); j++)
          std::cout << info.sampleRates[j] << " ";
      }
      std::cout << std::endl;
      if ( info.preferredSampleRate == 0 )
        std::cout << "No preferred sample rate found!" << std::endl;
      else
        std::cout << "Preferred sample rate = " << info.preferredSampleRate << std::endl;
    }
  }
  std::cout << std::endl;
}