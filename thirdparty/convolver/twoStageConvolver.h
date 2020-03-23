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

#ifndef _FFTCONVOLVER_TWOSTAGEFFTCONVOLVER_H
#define _FFTCONVOLVER_TWOSTAGEFFTCONVOLVER_H

#include "./convolver.h"
#include "./util.h"


namespace fftconvolver {
  /**
  * @class TwoStageFFTConvolver
  * @brief FFT convolver using two different block sizes
  *
  * The 2-stage convolver consists internally of two convolvers:
  *
  * - A head convolver, which processes the only the begin of the impulse response.
  *
  * - A tail convolver, which processes the rest and major amount of the impulse response.
  *
  * Using a short block size for the head convolver and a long block size for
  * the tail convolver results in much less CPU usage, while keeping the
  * calculation time of each processing call short.
  *
  * Furthermore, this convolver class provides virtual methods which provide the
  * possibility to move the tail convolution into the background (e.g. by using
  * multithreading, see startBackgroundProcessing()/waitForBackgroundProcessing()).
  *
  * As well as the basic FFTConvolver class, the 2-stage convolver is suitable
  * for real-time processing which means that no "unpredictable" operations like
  * allocations, locking, API calls, etc. are performed during processing (all
  * necessary allocations and preparations take place during initialization).
  */
  class TwoStageFFTConvolver {
  public:
    TwoStageFFTConvolver() :
      _headBlockSize(0), _tailBlockSize(0), _headConvolver(),
      _tailConvolver0(), _tailOutput0(), _tailPrecalculated0(0),
      _tailConvolver(), _tailOutput(), _tailPrecalculated(0),
      _tailInput(), _tailInputFill(0), _precalculatedPos(0),
      _backgroundProcessingInput() {}

    virtual ~TwoStageFFTConvolver() {
      reset();
    }

    /**
    * @brief Initialization the convolver
    * @param headBlockSize The head block size
    * @param tailBlockSize the tail block size
    * @param ir The impulse response
    * @param irLen Length of the impulse response in samples
    * @return true: Success - false: Failed
    */
    bool init(size_t headBlockSize, size_t tailBlockSize, const Sample* ir, size_t irLen) {
      reset();

      if (headBlockSize == 0 || tailBlockSize == 0) {
        return false;
      }

      headBlockSize = std::max(size_t(1), headBlockSize);
      if (headBlockSize > tailBlockSize) {
        assert(false);
        std::swap(headBlockSize, tailBlockSize);
      }

      // Ignore zeros at the end of the impulse response because they only waste computation time
      while (irLen > 0 && ::fabs(ir[irLen - 1]) < 0.000001f) {
        --irLen;
      }

      if (irLen == 0) {
        return true;
      }

      _headBlockSize = NextPowerOf2(headBlockSize);
      _tailBlockSize = NextPowerOf2(tailBlockSize);

      const size_t headIrLen = std::min(irLen, _tailBlockSize);
      _headConvolver.init(_headBlockSize, ir, headIrLen);

      if (irLen > _tailBlockSize) {
        const size_t conv1IrLen = std::min(irLen - _tailBlockSize, _tailBlockSize);
        _tailConvolver0.init(_headBlockSize, ir + _tailBlockSize, conv1IrLen);
        _tailOutput0.resize(_tailBlockSize);
        _tailPrecalculated0.resize(_tailBlockSize);
      }

      if (irLen > 2 * _tailBlockSize) {
        const size_t tailIrLen = irLen - (2 * _tailBlockSize);
        _tailConvolver.init(_tailBlockSize, ir + (2 * _tailBlockSize), tailIrLen);
        _tailOutput.resize(_tailBlockSize);
        _tailPrecalculated.resize(_tailBlockSize);
        _backgroundProcessingInput.resize(_tailBlockSize);
      }

      if (_tailPrecalculated0.size() > 0 || _tailPrecalculated.size() > 0) {
        _tailInput.resize(_tailBlockSize);
      }
      _tailInputFill = 0;
      _precalculatedPos = 0;

      return true;
    }

    /**
    * @brief Convolves the the given input samples and immediately outputs the result
    * @param input The input samples
    * @param output The convolution result
    * @param len Number of input/output samples
    */
    void process(const Sample* input, Sample* output, size_t len) {
      // Head
      _headConvolver.process(input, output, len);

      // Tail
      if (_tailInput.size() > 0) {
        size_t processed = 0;
        while (processed < len) {
          const size_t remaining = len - processed;
          const size_t processing = std::min(remaining, _headBlockSize - (_tailInputFill % _headBlockSize));
          assert(_tailInputFill + processing <= _tailBlockSize);

          // Sum head and tail
          const size_t sumBegin = processed;
          const size_t sumEnd = processed + processing;
          {
            // Sum: 1st tail block
            if (_tailPrecalculated0.size() > 0) {
              size_t precalculatedPos = _precalculatedPos;
              for (size_t i = sumBegin; i < sumEnd; ++i) {
                output[i] += _tailPrecalculated0[precalculatedPos];
                ++precalculatedPos;
              }
            }

            // Sum: 2nd-Nth tail block
            if (_tailPrecalculated.size() > 0) {
              size_t precalculatedPos = _precalculatedPos;
              for (size_t i = sumBegin; i < sumEnd; ++i) {
                output[i] += _tailPrecalculated[precalculatedPos];
                ++precalculatedPos;
              }
            }
            _precalculatedPos += processing;
          }

          // Fill input buffer for tail convolution
          ::memcpy(_tailInput.data() + _tailInputFill, input + processed, processing * sizeof(Sample));
          _tailInputFill += processing;
          assert(_tailInputFill <= _tailBlockSize);

          // Convolution: 1st tail block
          if (_tailPrecalculated0.size() > 0 && _tailInputFill % _headBlockSize == 0) {
            assert(_tailInputFill >= _headBlockSize);
            const size_t blockOffset = _tailInputFill - _headBlockSize;
            _tailConvolver0.process(_tailInput.data() + blockOffset, _tailOutput0.data() + blockOffset, _headBlockSize);
            if (_tailInputFill == _tailBlockSize) {
              SampleBuffer::Swap(_tailPrecalculated0, _tailOutput0);
            }
          }

          // Convolution: 2nd-Nth tail block (might be done in some background thread)
          if (_tailPrecalculated.size() > 0 &&
            _tailInputFill == _tailBlockSize &&
            _backgroundProcessingInput.size() == _tailBlockSize &&
            _tailOutput.size() == _tailBlockSize)
          {
            waitForBackgroundProcessing();
            SampleBuffer::Swap(_tailPrecalculated, _tailOutput);
            _backgroundProcessingInput.copyFrom(_tailInput);
            startBackgroundProcessing();
          }

          if (_tailInputFill == _tailBlockSize) {
            _tailInputFill = 0;
            _precalculatedPos = 0;
          }

          processed += processing;
        }
      }
    }

    /**
    * @brief Resets the convolver and discards the set impulse response
    */
    void reset() {
      _headBlockSize = 0;
      _tailBlockSize = 0;
      _headConvolver.reset();
      _tailConvolver0.reset();
      _tailOutput0.clear();
      _tailPrecalculated0.clear();
      _tailConvolver.reset();
      _tailOutput.clear();
      _tailPrecalculated.clear();
      _tailInput.clear();
      _tailInputFill = 0;
      _tailInputFill = 0;
      _precalculatedPos = 0;
      _backgroundProcessingInput.clear();
    }

  protected:
    /**
    * @brief Method called by the convolver if work for background processing is available
    *
    * The default implementation just calls doBackgroundProcessing() to perform the "bulk"
    * convolution. However, if you want to perform the majority of work in some background
    * thread (which is recommended), you can overload this method and trigger the execution
    * of doBackgroundProcessing() really in some background thread.
    */
    virtual void startBackgroundProcessing() {
      doBackgroundProcessing();
    }

    /**
    * @brief Called by the convolver if it expects the result of its previous call to startBackgroundProcessing()
    *
    * After returning from this method, all background processing has to be completed.
    */
    virtual void waitForBackgroundProcessing() {}

    /**
    * @brief Actually performs the background processing work
    */
    void doBackgroundProcessing() {
      _tailConvolver.process(_backgroundProcessingInput.data(), _tailOutput.data(), _tailBlockSize);
    }

  private:
    size_t _headBlockSize;
    size_t _tailBlockSize;
    FFTConvolver _headConvolver;
    FFTConvolver _tailConvolver0;
    SampleBuffer _tailOutput0;
    SampleBuffer _tailPrecalculated0;
    FFTConvolver _tailConvolver;
    SampleBuffer _tailOutput;
    SampleBuffer _tailPrecalculated;
    SampleBuffer _tailInput;
    size_t _tailInputFill;
    size_t _precalculatedPos;
    SampleBuffer _backgroundProcessingInput;

    // Prevent uncontrolled usage
    TwoStageFFTConvolver(const TwoStageFFTConvolver&);
    TwoStageFFTConvolver& operator=(const TwoStageFFTConvolver&);
  };

} // End of namespace fftconvolver

#endif // Header guard
