#pragma once

#ifndef SOUNDWOOFER_CUSTOM_WAVE
  #define DR_WAV_IMPLEMENTATION
  #include "./dependencies/dr_wav.h"
#endif

#include "./soundwooferFile.h"
#include "./soundwooferTypes.h"

namespace soundwoofer {
  namespace wave {
    template <typename Tin, typename Tout>
    /**
     * Simple since resampler, still needs a lowpass to avoid aliasing
     * Based on https://math.stackexchange.com/a/1723679
     */
    class WindowedSincResampler {
      double mStepSize = 0;
      size_t mWindowSize = 0;
    public:
      WindowedSincResampler(const double inFreq, const double outFreq, const size_t windowSize = 24) {
        mStepSize = inFreq / outFreq; // the step size in the input signal
        mWindowSize = windowSize;
      }

      size_t resample(Tin* in, const size_t length, Tout** out) {
        const size_t outSamples = std::floor(length / mStepSize);
        if (outSamples <= 0) { return 0; }
        Tout amplitude = mStepSize; // need to compensate for change in amplitude
        (*out) = new Tout[outSamples];
        double j = 0; // The position at which the sinc interpolation of the input signal should be evaluated
        for (size_t k = 0; k < outSamples; k++, j += mStepSize) {
          Tout interpolated = 0;
          const size_t lower = std::max(static_cast<size_t>(0), static_cast<size_t>(j - (mWindowSize / 2)));
          const size_t upper = std::min(length, static_cast<size_t>(j + mWindowSize / 2));
          for (size_t i = lower; i < upper; i++) {
            interpolated += in[i] * sinc(j - i);
          }
          (*out)[k] = interpolated * amplitude;
        }
        return outSamples;
      }

      static Tout lerp(const double i, Tin* buf) {
        const size_t lower = std::floor(i);
        const double a = i - lower;
        return (buf[lower] * (1 - a) + buf[lower + 1] * a);
      }

      static Tout sinc(const double i) {
        if (i == 0) {
          return 1;
        }
        return (sin(PI * i) / (PI * i));
      }
    };

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
      const float mix = 0.2;

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
    Status decodeWave(SWImpulseShared& ir, drwav* wav, size_t sampleRate, bool normalize) {
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
      for (int c = 0; c < ir->channels; c++) {
        ir->samples[c] = new float[ir->length];
      }

      // slow deinterleaving, but it works
      for (size_t s = 0; s < ir->length * ir->channels; s++) {
        const int channel = s % ir->channels;
        const size_t sample = s / ir->channels;
        ir->samples[channel][sample] = pSampleData[s];
      }

      free(pSampleData); // Free the interleaved buffer

      if (normalize) {
        normalizeIR(ir);
      }

      // Do resampling if desired
      if (0 < sampleRate && ir->sampleRate != sampleRate) {
        WindowedSincResampler<float, float> resampler(ir->sampleRate, sampleRate);
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