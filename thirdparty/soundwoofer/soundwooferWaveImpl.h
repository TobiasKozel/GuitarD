#pragma once

#include "./soundwooferWave.h"

#ifndef SOUNDWOOFER_CUSTOM_WAVE
  #define DR_WAV_IMPLEMENTATION
  #include "./dependencies/dr_wav.h"
#endif
#include "./soundwooferResampler.h"
#include <algorithm>
#include <cmath>
#include "./soundwooferFile.h"


namespace soundwoofer {
  namespace wave {
    void normalizeIR(SWImpulseShared& ir) {
      if (ir->normalized || ir->samples == nullptr) { return; }
      float sum = 0;
      float peak = 0;
      for (size_t s = 0; s < ir->length; s++) {
        for (size_t c = 0; c < ir->channels; c++) {
          const float val = std::abs(ir->samples[c][s]);
          sum += val;
          peak = std::max(peak, val);
        }
      }

      /**
       * Some arbitrary weighting of peak and sum
       * which seemed to work well
       */
      sum = sum / static_cast<float>(ir->channels);
      sum = (1.f / sum) * 4;
      peak = (1.f / peak) * 0.9; // bit of headroom
      const float mix = 0.1;

      for (size_t s = 0; s < ir->length; s++) {
        for (size_t c = 0; c < ir->channels; c++) {
          ir->samples[c][s] *= (peak * mix) + (sum * (1 - mix));
        }
      }

      ir->normalized = true;
    }

#ifndef SOUNDWOOFER_CUSTOM_WAVE
    /**
     * decodes, deinterleaves and resamples the wave
     */
    Status decodeWave(SWImpulseShared& ir, void* dwav, size_t sampleRate, bool normalize) {
      drwav* wav = reinterpret_cast<drwav*>(dwav);
      // interleaved buffer from drwav
      float* pSampleData = static_cast<float*>(malloc(
        static_cast<size_t>(wav->totalPCMFrameCount)* wav->channels * sizeof(float)
      ));

      // Fill the buffer
      ir->length = drwav_read_pcm_frames_f32(wav, wav->totalPCMFrameCount, pSampleData);
      if (ir->length == 0) { return WAV_ERROR; }

      ir->sampleRate = wav->sampleRate;
      ir->channels = wav->channels;

      // Deinterleaved buffer
      ir->samples = new float* [ir->channels];
      for (size_t c = 0; c < ir->channels; c++) {
        ir->samples[c] = new float[ir->length];
      }

      // slow deinterleaving, but it works
      for (size_t s = 0; s < ir->length * ir->channels; s++) {
        const size_t channel = s % ir->channels;
        const size_t sample = s / ir->channels;
        ir->samples[channel][sample] = pSampleData[s];
      }

      free(pSampleData); // Free the interleaved buffer

      if (normalize) {
        normalizeIR(ir);
      }

      // Do resampling if desired
      if (0 < sampleRate && ir->sampleRate != sampleRate) {
        Resampler resampler(ir->sampleRate, sampleRate);
        float** resampled = new float* [ir->channels];
        size_t sampleCount = 0;
        const size_t channelCount = ir->channels;
        for (size_t i = 0; i < channelCount; i++) {
          sampleCount = resampler.resample(
            ir->samples[i], ir->length, &(resampled[i])
          );
        }
        ir->clearSamples(); // get rid of the original buffer
        ir->length = sampleCount; // size changed
        ir->channels = channelCount; // This was also reset in clearSamples()
        ir->samples = resampled; // new buffers
        ir->sampleRate = sampleRate;
      }
      return SUCCESS;
    }

    /**
     * Loads a wave file from an absolute path
     */
    Status loadWaveFile(SWImpulseShared& ir, std::string absolutePath, size_t sampleRate, bool normalize) {
      drwav wav;

      // Load the Wave file
      if (!drwav_init_file(&wav, absolutePath.c_str(), nullptr)) {
        if (ir->source == SOUNDWOOFER_SRC) {
          return NOT_CACHED; // This means we'll need to go online and get the IR
        }
        return WAV_ERROR; // Means the wave is probably not there and there's not way to get it
        // TODO try looking for it by using the hash

      }

      if (ir->source == USER_SRC) {
        // Hash the file so we can go look for it if the file is missing on load
        ir->id = file::hashFile(absolutePath);
      }

      const Status decodeStatus = decodeWave(ir, &wav, sampleRate, normalize);
      drwav_uninit(&wav);
      return decodeStatus;
    }

    /**
     * Loads a wave from memory
     */
    Status loadWaveMemory(SWImpulseShared& ir, const char* waveData, const size_t length, size_t sampleRate, bool normalize) {
      drwav wav;
      if (!drwav_init_memory(&wav, waveData, length, nullptr)) {
        return WAV_ERROR;
      }
      const Status decodeStatus = decodeWave(ir, &wav, sampleRate, normalize);
      drwav_uninit(&wav);
      return decodeStatus;
    }
#endif

  }
}