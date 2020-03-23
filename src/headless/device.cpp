/**
 * Simple cli app which uses mini audio to do audio io
 */


#define SAMPLE_TYPE_FLOAT
#define GUITARD_SSE

#include <fstream>
#include <string>
#include "./GHeadless.h"

#define MINIAUDIO_IMPLEMENTATION
#include "../../thirdparty/miniaudio.h"

guitard::GuitarDHeadless headless;

#define CHANNEL_COUNT 2
guitard::sample* in[CHANNEL_COUNT] = { nullptr };
guitard::sample* out[CHANNEL_COUNT] = { nullptr };

void data_callback(ma_device* pDevice, void* pOutput, const void* pInput, ma_uint32 frameCount)
{
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
  ma_result result;
  ma_device_config deviceConfig;
  ma_device device;

  deviceConfig = ma_device_config_init(ma_device_type_duplex);
  deviceConfig.capture.pDeviceID  = NULL;
  deviceConfig.capture.format     = ma_format_f32;
  deviceConfig.capture.channels   = CHANNEL_COUNT;
  deviceConfig.capture.shareMode  = ma_share_mode_shared;
  deviceConfig.playback.pDeviceID = NULL;
  deviceConfig.playback.format    = ma_format_f32;
  deviceConfig.playback.channels  = CHANNEL_COUNT;
  deviceConfig.dataCallback       = data_callback;
  result = ma_device_init(NULL, &deviceConfig, &device);
  if (result != MA_SUCCESS) {
    return result;
  }

  for (int i = 0; i < CHANNEL_COUNT; i++) {
    in[i] = new guitard::sample[GUITARD_MAX_BUFFER];
    out[i] = new guitard::sample[GUITARD_MAX_BUFFER];
  }

  headless.setConfig(device.sampleRate, device.playback.channels, device.capture.channels);

  std::ifstream file("./src/headless/preset.json");
  std::string contents((std::istreambuf_iterator<char>(file)), std::istreambuf_iterator<char>());
  headless.load(contents.c_str());

  ma_device_start(&device);
  
  printf("Press Enter to quit...\n");
  getchar();
  ma_device_uninit(&device);

  (void)argc;
  (void)argv;
  return 0;
}