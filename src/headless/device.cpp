#include <stdio.h>

#define SAMPLE_TYPE_FLOAT
#define GUITARD_SSE
#define SOUNDWOOFER_NO_API

#include <fstream>
#include <string>
#include "./GHeadless.h"

#define MINIAUDIO_IMPLEMENTATION
#define MA_NO_DECODING
#include "../../thirdparty/miniaudio.h"

guitard::GuitarDHeadless headless;

#define CHANNEL_COUNT 2
#define MAX_BLOCK_SIZE 1024

guitard::sample buffers[CHANNEL_COUNT * 2][MAX_BLOCK_SIZE];
guitard::sample* in[CHANNEL_COUNT] = { buffers[0], buffers[1] };
guitard::sample* out[CHANNEL_COUNT] = { buffers[2], buffers[3] };

void data_callback(ma_device* pDevice, void* pOutput, const void* pInput, ma_uint32 frameCount) {
  MA_ASSERT(pDevice->capture.format == pDevice->playback.format);
  MA_ASSERT(pDevice->capture.channels == pDevice->playback.channels);
  const guitard::sample* inF = static_cast<const guitard::sample*>(pInput);
  guitard::sample* outF = static_cast<guitard::sample*>(pOutput);
  for (unsigned int s = 0; s < frameCount * CHANNEL_COUNT; s++) {
    const unsigned int channel = s % CHANNEL_COUNT;
    const unsigned int sample = s / CHANNEL_COUNT;
    in[channel][sample] = inF[s];
  }
  headless.process(in, out, frameCount);
  for (int i = 0, j = 0; i < frameCount; i++, j += CHANNEL_COUNT) {
    outF[j] = out[0][i];
    outF[j + 1] = out[1][i];
  }
}

int main(int argc, char** argv) {
  ma_uint32 frame_size = 128;
  const char* path = "../../thirdparty/soundwoofer/dummy_backend/presets/";
  if (argc > 1) {
    frame_size = atoi(argv[1]);
  }
  if (argc == 2) {
    path = argv[2];
  }

  std::vector<soundwoofer::file::FileInfo> presets = soundwoofer::file::scanDir(path);
  int currentPresetIndex = 0;

  if (presets.empty()) { return -1; }

  ma_result result;
  ma_device_config deviceConfig;
  ma_device device;
  deviceConfig = ma_device_config_init(ma_device_type_duplex);
  deviceConfig.capture.pDeviceID  = NULL;
  deviceConfig.capture.format     = ma_format_f32;
  deviceConfig.capture.channels   = CHANNEL_COUNT;
  deviceConfig.capture.shareMode  = ma_share_mode_exclusive;
  deviceConfig.playback.pDeviceID = NULL;
  deviceConfig.playback.format    = ma_format_f32;
  deviceConfig.playback.channels  = CHANNEL_COUNT;
  deviceConfig.dataCallback       = data_callback;
  deviceConfig.periodSizeInFrames = frame_size;
  result = ma_device_init(NULL, &deviceConfig, &device);
  if (result != MA_SUCCESS) {
      return result;
  }
  printf("\n Framesize: %u\n", deviceConfig.periodSizeInFrames);
	headless.setConfig(device.sampleRate, device.playback.channels, device.capture.channels);

  ma_device_start(&device);

  while (42) {
    std::ifstream file(presets[currentPresetIndex].absolute);
    std::string contents((std::istreambuf_iterator<char>(file)), std::istreambuf_iterator<char>());
    headless.load(contents.c_str());
    printf("Loaded Preset %i ", currentPresetIndex);
    printf("%s\n", presets[currentPresetIndex].name.c_str());
    printf("Press to load the next preset\n");
    getchar();
    currentPresetIndex++;
    if (currentPresetIndex <= presets.size()) {
      currentPresetIndex = 0;
    }
  }

  ma_device_uninit(&device);
  (void)argc;
  (void)argv;
  return 0;
}
