// ==================================================================================
// Copyright (c) 2017 HiFi-LoFi
//
// Permission is hereby granted, free of charge, to any person obtaining a copy
// of this software and associated documentation files (the "Software"), to deal
// in the Software without restriction, including without limitation the rights
// to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
// copies of the Software, and to permit persons to whom the Software is furnished
// to do so, subject to the following conditions:
//
// The above copyright notice and this permission notice shall be included in
// all copies or substantial portions of the Software.
//
// THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
// IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY, FITNESS
// FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR
// COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER
// IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION
// WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.
// ==================================================================================
// NOTE: Modified for GuitarD to header only

#ifndef _FFTCONVOLVER_FFTCONVOLVER_H
#define _FFTCONVOLVER_FFTCONVOLVER_H

#include "./fft.h"
#include "./util.h"

#if defined (FFTCONVOLVER_USE_SSE)
  #include <xmmintrin.h>
#endif
#include <vector>
#include <cassert>
#include <cmath>


namespace fftconvolver {
  /**
  * @class FFTConvolver
  * @brief Implementation of a partitioned FFT convolution algorithm with uniform block size
  *
  * Some notes on how to use it:
  *
  * - After initialization with an impulse response, subsequent data portions of
  *   arbitrary length can be convolved. The convolver internally can handle
  *   this by using appropriate buffering.
  *
  * - The convolver works without "latency" (except for the required
  *   processing time, of course), i.e. the output always is the convolved
  *   input for each processing call.
  *
  * - The convolver is suitable for real-time processing which means that no
  *   "unpredictable" operations like allocations, locking, API calls, etc. are
  *   performed during processing (all necessary allocations and preparations take
  *   place during initialization).
  */
  class FFTConvolver {
  public:
    FFTConvolver() :
      _blockSize(0), _segSize(0), _segCount(0),
      _fftComplexSize(0), _segments(), _segmentsIR(),
      _fftBuffer(), _fft(), _preMultiplied(), _conv(),
      _overlap(), _current(0), _inputBuffer(), _inputBufferFill(0) {}

    virtual ~FFTConvolver() {
       reset();
    }

    /**
    * @brief Initializes the convolver
    * @param blockSize Block size internally used by the convolver (partition size)
    * @param ir The impulse response
    * @param irLen Length of the impulse response
    * @return true: Success - false: Failed
    */
    bool init(size_t blockSize, const Sample* ir, size_t irLen) {
      reset();

      if (blockSize == 0) {
        return false;
      }

      //// Ignore zeros at the end of the impulse response because they only waste computation time
      while (irLen > 0 && ::fabs(ir[irLen - 1]) < 0.000001f) {
        --irLen;
      }

      if (irLen == 0) {
        return true;
      }

      _blockSize = NextPowerOf2(blockSize);
      _segSize = 2 * _blockSize;
      _segCount = static_cast<size_t>(::ceil(static_cast<float>(irLen) / static_cast<float>(_blockSize)));
      _fftComplexSize = audiofft::AudioFFT::ComplexSize(_segSize);

      // FFT
      _fft.init(_segSize);
      _fftBuffer.resize(_segSize);

      // Prepare segments
      for (size_t i = 0; i < _segCount; ++i) {
        _segments.push_back(new SplitComplex(_fftComplexSize));
      }

      // Prepare IR
      for (size_t i = 0; i < _segCount; ++i) {
        SplitComplex* segment = new SplitComplex(_fftComplexSize);
        const size_t remaining = irLen - (i * _blockSize);
        const size_t sizeCopy = (remaining >= _blockSize) ? _blockSize : remaining;
        CopyAndPad(_fftBuffer, &ir[i * _blockSize], sizeCopy);
        _fft.fft(_fftBuffer.data(), segment->re(), segment->im());
        _segmentsIR.push_back(segment);
      }

      // Prepare convolution buffers  
      _preMultiplied.resize(_fftComplexSize);
      _conv.resize(_fftComplexSize);
      _overlap.resize(_blockSize);

      // Prepare input buffer
      _inputBuffer.resize(_blockSize);
      _inputBufferFill = 0;

      // Reset current position
      _current = 0;

      return true;
    }

    /**
    * @brief Convolves the the given input samples and immediately outputs the result
    * @param input The input samples
    * @param output The convolution result
    * @param len Number of input/output samples
    */
    void process(const Sample* input, Sample* output, size_t len) {
      if (_segCount == 0) {
        ::memset(output, 0, len * sizeof(Sample));
        return;
      }

      size_t processed = 0;
      while (processed < len) {
        const bool inputBufferWasEmpty = (_inputBufferFill == 0);
        const size_t processing = std::min(len - processed, _blockSize - _inputBufferFill);
        const size_t inputBufferPos = _inputBufferFill;
        ::memcpy(_inputBuffer.data() + inputBufferPos, input + processed, processing * sizeof(Sample));

        // Forward FFT
        CopyAndPad(_fftBuffer, &_inputBuffer[0], _blockSize);
        _fft.fft(_fftBuffer.data(), _segments[_current]->re(), _segments[_current]->im());

        // Complex multiplication
        if (inputBufferWasEmpty) {
          _preMultiplied.setZero();
          for (size_t i = 1; i < _segCount; ++i) {
            const size_t indexIr = i;
            const size_t indexAudio = (_current + i) % _segCount;
            ComplexMultiplyAccumulate(_preMultiplied, *_segmentsIR[indexIr], *_segments[indexAudio]);
          }
        }
        _conv.copyFrom(_preMultiplied);
        ComplexMultiplyAccumulate(_conv, *_segments[_current], *_segmentsIR[0]);

        // Backward FFT
        _fft.ifft(_fftBuffer.data(), _conv.re(), _conv.im());

        // Add overlap
        Sum(output + processed, _fftBuffer.data() + inputBufferPos, _overlap.data() + inputBufferPos, processing);

        // Input buffer full => Next block
        _inputBufferFill += processing;
        if (_inputBufferFill == _blockSize) {
          // Input buffer is empty again now
          _inputBuffer.setZero();
          _inputBufferFill = 0;

          // Save the overlap
          ::memcpy(_overlap.data(), _fftBuffer.data() + _blockSize, _blockSize * sizeof(Sample));

          // Update current segment
          _current = (_current > 0) ? (_current - 1) : (_segCount - 1);
        }
        processed += processing;
      }
    }

    /**
    * @brief Resets the convolver and discards the set impulse response
    */
    void reset() {
      for (size_t i = 0; i < _segCount; ++i) {
        delete _segments[i];
        delete _segmentsIR[i];
      }

      _blockSize = 0;
      _segSize = 0;
      _segCount = 0;
      _fftComplexSize = 0;
      _segments.clear();
      _segmentsIR.clear();
      _fftBuffer.clear();
      _fft.init(0);
      _preMultiplied.clear();
      _conv.clear();
      _overlap.clear();
      _current = 0;
      _inputBuffer.clear();
      _inputBufferFill = 0;
    }

  private:
    size_t _blockSize;
    size_t _segSize;
    size_t _segCount;
    size_t _fftComplexSize;
    std::vector<SplitComplex*> _segments;
    std::vector<SplitComplex*> _segmentsIR;
    SampleBuffer _fftBuffer;
    audiofft::AudioFFT _fft;
    SplitComplex _preMultiplied;
    SplitComplex _conv;
    SampleBuffer _overlap;
    size_t _current;
    SampleBuffer _inputBuffer;
    size_t _inputBufferFill;

    // Prevent uncontrolled usage
    FFTConvolver(const FFTConvolver&);
    FFTConvolver& operator=(const FFTConvolver&);
  };

} // End of namespace fftconvolver

#endif // Header guard
