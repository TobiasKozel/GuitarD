#include <stdio.h>
#include <fstream>
#include <string>
#include <iostream>
#include <cstdlib>
#include <cstring>
#include <thread>

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

/**
 * Takes a few arguments
 * First one is
 */
int main(int argc, char** argv) {
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
