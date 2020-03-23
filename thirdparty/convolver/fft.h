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

#ifndef _AUDIOFFT_H
#define _AUDIOFFT_H


/**
* AudioFFT provides real-to-complex/complex-to-real FFT routines.
*
* Features:
*
* - Real-complex FFT and complex-real inverse FFT for power-of-2-sized real data.
*
* - Uniform interface to different FFT implementations (currently Ooura, FFTW3 and Apple Accelerate).
*
* - Complex data is handled in "split-complex" format, i.e. there are separate
*   arrays for the real and imaginary parts which can be useful for SIMD optimizations
*   (split-complex arrays have to be of length (size/2+1) representing bins from DC
*   to Nyquist frequency).
*
* - Output is "ready to use" (all scaling etc. is already handled internally).
*
* - No allocations/deallocations after the initialization which makes it usable
*   for real-time audio applications (that's what I wrote it for and using it).
*
*
* How to use it in your project:
*
* - Add the .h and .cpp file to your project - that's all.
*
* - To get extra speed, you can link FFTW3 to your project and define
*   AUDIOFFT_FFTW3 (however, please check whether your project suits the
*   according license).
*
* - To get the best speed on Apple platforms, you can link the Apple
*   Accelerate framework to your project and define
*   AUDIOFFT_APPLE_ACCELERATE  (however, please check whether your
*   project suits the according license).
*
*
* Remarks:
*
* - AudioFFT is not intended to be the fastest FFT, but to be a fast-enough
*   FFT suitable for most audio applications.
*
* - AudioFFT uses the quite liberal MIT license.
*
*
* Example usage:
* @code
* #include "AudioFFT.h"
*
* void Example()
* {
*   const size_t fftSize = 1024; // Needs to be power of 2!
*
*   std::vector<Sample> input(fftSize, 0.0f);
*   std::vector<Sample> re(audiofft::AudioFFT::ComplexSize(fftSize));
*   std::vector<Sample> im(audiofft::AudioFFT::ComplexSize(fftSize));
*   std::vector<Sample> output(fftSize);
*
*   audiofft::AudioFFT fft;
*   fft.init(1024);
*   fft.fft(input.data(), re.data(), im.data());
*   fft.ifft(output.data(), re.data(), im.data());
* }
* @endcode
*
* NOTE: Modified for GuitarD to header only and threw out the FFTW implementation
*       because of the license
*/


#include <cstddef>
#include <memory>
#include "./util.h"
#include <cassert>
#include <cmath>

#if defined(AUDIOFFT_APPLE_ACCELERATE)
  #define AUDIOFFT_APPLE_ACCELERATE_USED
  #include <Accelerate/Accelerate.h>
  #include <vector>
#else
  #if !defined(AUDIOFFT_OOURA)
    #define AUDIOFFT_OOURA
  #endif
  #define AUDIOFFT_OOURA_USED
  #include <vector>
#endif

namespace audiofft {

  namespace detail {
    class AudioFFTImpl {
    public:
      AudioFFTImpl() = default;
      AudioFFTImpl(const AudioFFTImpl&) = delete;
      AudioFFTImpl& operator=(const AudioFFTImpl&) = delete;
      virtual ~AudioFFTImpl() = default;
      virtual void init(size_t size) = 0;
      virtual void fft(const fftconvolver::Sample * data, fftconvolver::Sample * re, fftconvolver::Sample * im) = 0;
      virtual void ifft(fftconvolver::Sample * data, const fftconvolver::Sample * re, const fftconvolver::Sample * im) = 0;
    };

    constexpr bool IsPowerOf2(size_t val) {
      return (val == 1 || (val & (val - 1)) == 0);
    }

    template<typename TypeDest, typename TypeSrc>
    void ConvertBuffer(TypeDest* dest, const TypeSrc* src, const size_t len) {
      for (size_t i = 0; i < len; ++i) {
        dest[i] = static_cast<TypeDest>(src[i]);
      }
    }

    template<typename TypeDest, typename TypeSrc, typename TypeFactor>
    void ScaleBuffer(TypeDest* dest, const TypeSrc* src, const TypeFactor factor, const size_t len) {
      for (size_t i = 0; i < len; ++i) {
        dest[i] = static_cast<TypeDest>(static_cast<TypeFactor>(src[i])* factor);
      }
    }
  }


  // =============================================================

    // ================================================================


#ifdef AUDIOFFT_OOURA_USED
  /**
   * @internal
   * @class OouraFFT
   * @brief FFT implementation based on the great radix-4 routines by Takuya Ooura
   */
  class OouraFFT : public detail::AudioFFTImpl {
  public:
    OouraFFT() : detail::AudioFFTImpl(), _size(0), _ip(), _w(), _buffer() {}

    OouraFFT(const OouraFFT&) = delete;
    OouraFFT& operator=(const OouraFFT&) = delete;

    void init(size_t size) override {
      if (_size != size) {
        _ip.resize(2 + static_cast<int>(std::sqrt(static_cast<double>(size))));
        _w.resize(size / 2);
        _buffer.resize(size);
        _size = size;

        const int size4 = static_cast<int>(_size) / 4;
        makewt(size4, _ip.data(), _w.data());
        makect(size4, _ip.data(), _w.data() + size4);
      }
    }

    void fft(const fftconvolver::Sample* data, fftconvolver::Sample* re, fftconvolver::Sample* im) override {
      // Convert into the format as required by the Ooura FFT
      detail::ConvertBuffer(_buffer.data(), data, _size);

      rdft(static_cast<int>(_size), +1, _buffer.data(), _ip.data(), _w.data());

      // Convert back to split-complex
      {
        double* b = _buffer.data();
        double* bEnd = b + _size;
        fftconvolver::Sample* r = re;
        fftconvolver::Sample* i = im;
        while (b != bEnd) {
          *(r++) = static_cast<fftconvolver::Sample>(*(b++));
          *(i++) = static_cast<fftconvolver::Sample>(-(*(b++)));
        }
      }
      const size_t size2 = _size / 2;
      re[size2] = -im[0];
      im[0] = 0.0;
      im[size2] = 0.0;
    }

    void ifft(fftconvolver::Sample* data, const fftconvolver::Sample* re, const fftconvolver::Sample* im) override {
      // Convert into the format as required by the Ooura FFT
      {
        double* b = _buffer.data();
        double* bEnd = b + _size;
        const fftconvolver::Sample* r = re;
        const fftconvolver::Sample* i = im;
        while (b != bEnd) {
          *(b++) = static_cast<double>(*(r++));
          *(b++) = -static_cast<double>(*(i++));
        }
        _buffer[1] = re[_size / 2];
      }
      rdft(static_cast<int>(_size), -1, _buffer.data(), _ip.data(), _w.data());
      // Convert back to split-complex
      detail::ScaleBuffer(data, _buffer.data(), 2.0 / static_cast<double>(_size), _size);
    }

  private:
    size_t _size;
    std::vector<int> _ip;
    std::vector<double> _w;
    std::vector<double> _buffer;

    void rdft(int n, int isgn, double* a, int* ip, double* w) {
      int nw = ip[0];
      int nc = ip[1];

      if (isgn >= 0) {
        if (n > 4) {
          bitrv2(n, ip + 2, a);
          cftfsub(n, a, w);
          rftfsub(n, a, nc, w + nw);
        }
        else if (n == 4) {
          cftfsub(n, a, w);
        }
        double xi = a[0] - a[1];
        a[0] += a[1];
        a[1] = xi;
      }
      else {
        a[1] = 0.5 * (a[0] - a[1]);
        a[0] -= a[1];
        if (n > 4) {
          rftbsub(n, a, nc, w + nw);
          bitrv2(n, ip + 2, a);
          cftbsub(n, a, w);
        }
        else if (n == 4) {
          cftfsub(n, a, w);
        }
      }
    }

    /* -------- initializing routines -------- */
    void makewt(int nw, int* ip, double* w) {
      int j, nwh;
      double delta, x, y;

      ip[0] = nw;
      ip[1] = 1;
      if (nw > 2) {
        nwh = nw >> 1;
        delta = atan(1.0) / nwh;
        w[0] = 1;
        w[1] = 0;
        w[nwh] = cos(delta * nwh);
        w[nwh + 1] = w[nwh];
        if (nwh > 2) {
          for (j = 2; j < nwh; j += 2) {
            x = cos(delta * j);
            y = sin(delta * j);
            w[j] = x;
            w[j + 1] = y;
            w[nw - j] = y;
            w[nw - j + 1] = x;
          }
          bitrv2(nw, ip + 2, w);
        }
      }
    }

    void makect(int nc, int* ip, double* c) {
      int j, nch;
      double delta;

      ip[1] = nc;
      if (nc > 1) {
        nch = nc >> 1;
        delta = atan(1.0) / nch;
        c[0] = cos(delta * nch);
        c[nch] = 0.5 * c[0];
        for (j = 1; j < nch; j++) {
          c[j] = 0.5 * cos(delta * j);
          c[nc - j] = 0.5 * sin(delta * j);
        }
      }
    }

    /* -------- child routines -------- */
    void bitrv2(int n, int* ip, double* a) {
      int j, j1, k, k1, l, m, m2;
      double xr, xi, yr, yi;

      ip[0] = 0;
      l = n;
      m = 1;
      while ((m << 3) < l) {
        l >>= 1;
        for (j = 0; j < m; j++) {
          ip[m + j] = ip[j] + l;
        }
        m <<= 1;
      }
      m2 = 2 * m;
      if ((m << 3) == l) {
        for (k = 0; k < m; k++) {
          for (j = 0; j < k; j++) {
            j1 = 2 * j + ip[k];
            k1 = 2 * k + ip[j];
            xr = a[j1];
            xi = a[j1 + 1];
            yr = a[k1];
            yi = a[k1 + 1];
            a[j1] = yr;
            a[j1 + 1] = yi;
            a[k1] = xr;
            a[k1 + 1] = xi;
            j1 += m2;
            k1 += 2 * m2;
            xr = a[j1];
            xi = a[j1 + 1];
            yr = a[k1];
            yi = a[k1 + 1];
            a[j1] = yr;
            a[j1 + 1] = yi;
            a[k1] = xr;
            a[k1 + 1] = xi;
            j1 += m2;
            k1 -= m2;
            xr = a[j1];
            xi = a[j1 + 1];
            yr = a[k1];
            yi = a[k1 + 1];
            a[j1] = yr;
            a[j1 + 1] = yi;
            a[k1] = xr;
            a[k1 + 1] = xi;
            j1 += m2;
            k1 += 2 * m2;
            xr = a[j1];
            xi = a[j1 + 1];
            yr = a[k1];
            yi = a[k1 + 1];
            a[j1] = yr;
            a[j1 + 1] = yi;
            a[k1] = xr;
            a[k1 + 1] = xi;
          }
          j1 = 2 * k + m2 + ip[k];
          k1 = j1 + m2;
          xr = a[j1];
          xi = a[j1 + 1];
          yr = a[k1];
          yi = a[k1 + 1];
          a[j1] = yr;
          a[j1 + 1] = yi;
          a[k1] = xr;
          a[k1 + 1] = xi;
        }
      }
      else {
        for (k = 1; k < m; k++) {
          for (j = 0; j < k; j++) {
            j1 = 2 * j + ip[k];
            k1 = 2 * k + ip[j];
            xr = a[j1];
            xi = a[j1 + 1];
            yr = a[k1];
            yi = a[k1 + 1];
            a[j1] = yr;
            a[j1 + 1] = yi;
            a[k1] = xr;
            a[k1 + 1] = xi;
            j1 += m2;
            k1 += m2;
            xr = a[j1];
            xi = a[j1 + 1];
            yr = a[k1];
            yi = a[k1 + 1];
            a[j1] = yr;
            a[j1 + 1] = yi;
            a[k1] = xr;
            a[k1 + 1] = xi;
          }
        }
      }
    }


    void cftfsub(int n, double* a, double* w) {
      int j, j1, j2, j3, l;
      double x0r, x0i, x1r, x1i, x2r, x2i, x3r, x3i;

      l = 2;
      if (n > 8) {
        cft1st(n, a, w);
        l = 8;
        while ((l << 2) < n) {
          cftmdl(n, l, a, w);
          l <<= 2;
        }
      }
      if ((l << 2) == n) {
        for (j = 0; j < l; j += 2) {
          j1 = j + l;
          j2 = j1 + l;
          j3 = j2 + l;
          x0r = a[j] + a[j1];
          x0i = a[j + 1] + a[j1 + 1];
          x1r = a[j] - a[j1];
          x1i = a[j + 1] - a[j1 + 1];
          x2r = a[j2] + a[j3];
          x2i = a[j2 + 1] + a[j3 + 1];
          x3r = a[j2] - a[j3];
          x3i = a[j2 + 1] - a[j3 + 1];
          a[j] = x0r + x2r;
          a[j + 1] = x0i + x2i;
          a[j2] = x0r - x2r;
          a[j2 + 1] = x0i - x2i;
          a[j1] = x1r - x3i;
          a[j1 + 1] = x1i + x3r;
          a[j3] = x1r + x3i;
          a[j3 + 1] = x1i - x3r;
        }
      }
      else {
        for (j = 0; j < l; j += 2) {
          j1 = j + l;
          x0r = a[j] - a[j1];
          x0i = a[j + 1] - a[j1 + 1];
          a[j] += a[j1];
          a[j + 1] += a[j1 + 1];
          a[j1] = x0r;
          a[j1 + 1] = x0i;
        }
      }
    }


    void cftbsub(int n, double* a, double* w) {
      int j, j1, j2, j3, l;
      double x0r, x0i, x1r, x1i, x2r, x2i, x3r, x3i;

      l = 2;
      if (n > 8) {
        cft1st(n, a, w);
        l = 8;
        while ((l << 2) < n) {
          cftmdl(n, l, a, w);
          l <<= 2;
        }
      }
      if ((l << 2) == n) {
        for (j = 0; j < l; j += 2) {
          j1 = j + l;
          j2 = j1 + l;
          j3 = j2 + l;
          x0r = a[j] + a[j1];
          x0i = -a[j + 1] - a[j1 + 1];
          x1r = a[j] - a[j1];
          x1i = -a[j + 1] + a[j1 + 1];
          x2r = a[j2] + a[j3];
          x2i = a[j2 + 1] + a[j3 + 1];
          x3r = a[j2] - a[j3];
          x3i = a[j2 + 1] - a[j3 + 1];
          a[j] = x0r + x2r;
          a[j + 1] = x0i - x2i;
          a[j2] = x0r - x2r;
          a[j2 + 1] = x0i + x2i;
          a[j1] = x1r - x3i;
          a[j1 + 1] = x1i - x3r;
          a[j3] = x1r + x3i;
          a[j3 + 1] = x1i + x3r;
        }
      }
      else {
        for (j = 0; j < l; j += 2) {
          j1 = j + l;
          x0r = a[j] - a[j1];
          x0i = -a[j + 1] + a[j1 + 1];
          a[j] += a[j1];
          a[j + 1] = -a[j + 1] - a[j1 + 1];
          a[j1] = x0r;
          a[j1 + 1] = x0i;
        }
      }
    }

    void cft1st(int n, double* a, double* w) {
      int j, k1, k2;
      double wk1r, wk1i, wk2r, wk2i, wk3r, wk3i;
      double x0r, x0i, x1r, x1i, x2r, x2i, x3r, x3i;

      x0r = a[0] + a[2];
      x0i = a[1] + a[3];
      x1r = a[0] - a[2];
      x1i = a[1] - a[3];
      x2r = a[4] + a[6];
      x2i = a[5] + a[7];
      x3r = a[4] - a[6];
      x3i = a[5] - a[7];
      a[0] = x0r + x2r;
      a[1] = x0i + x2i;
      a[4] = x0r - x2r;
      a[5] = x0i - x2i;
      a[2] = x1r - x3i;
      a[3] = x1i + x3r;
      a[6] = x1r + x3i;
      a[7] = x1i - x3r;
      wk1r = w[2];
      x0r = a[8] + a[10];
      x0i = a[9] + a[11];
      x1r = a[8] - a[10];
      x1i = a[9] - a[11];
      x2r = a[12] + a[14];
      x2i = a[13] + a[15];
      x3r = a[12] - a[14];
      x3i = a[13] - a[15];
      a[8] = x0r + x2r;
      a[9] = x0i + x2i;
      a[12] = x2i - x0i;
      a[13] = x0r - x2r;
      x0r = x1r - x3i;
      x0i = x1i + x3r;
      a[10] = wk1r * (x0r - x0i);
      a[11] = wk1r * (x0r + x0i);
      x0r = x3i + x1r;
      x0i = x3r - x1i;
      a[14] = wk1r * (x0i - x0r);
      a[15] = wk1r * (x0i + x0r);
      k1 = 0;
      for (j = 16; j < n; j += 16) {
        k1 += 2;
        k2 = 2 * k1;
        wk2r = w[k1];
        wk2i = w[k1 + 1];
        wk1r = w[k2];
        wk1i = w[k2 + 1];
        wk3r = wk1r - 2 * wk2i * wk1i;
        wk3i = 2 * wk2i * wk1r - wk1i;
        x0r = a[j] + a[j + 2];
        x0i = a[j + 1] + a[j + 3];
        x1r = a[j] - a[j + 2];
        x1i = a[j + 1] - a[j + 3];
        x2r = a[j + 4] + a[j + 6];
        x2i = a[j + 5] + a[j + 7];
        x3r = a[j + 4] - a[j + 6];
        x3i = a[j + 5] - a[j + 7];
        a[j] = x0r + x2r;
        a[j + 1] = x0i + x2i;
        x0r -= x2r;
        x0i -= x2i;
        a[j + 4] = wk2r * x0r - wk2i * x0i;
        a[j + 5] = wk2r * x0i + wk2i * x0r;
        x0r = x1r - x3i;
        x0i = x1i + x3r;
        a[j + 2] = wk1r * x0r - wk1i * x0i;
        a[j + 3] = wk1r * x0i + wk1i * x0r;
        x0r = x1r + x3i;
        x0i = x1i - x3r;
        a[j + 6] = wk3r * x0r - wk3i * x0i;
        a[j + 7] = wk3r * x0i + wk3i * x0r;
        wk1r = w[k2 + 2];
        wk1i = w[k2 + 3];
        wk3r = wk1r - 2 * wk2r * wk1i;
        wk3i = 2 * wk2r * wk1r - wk1i;
        x0r = a[j + 8] + a[j + 10];
        x0i = a[j + 9] + a[j + 11];
        x1r = a[j + 8] - a[j + 10];
        x1i = a[j + 9] - a[j + 11];
        x2r = a[j + 12] + a[j + 14];
        x2i = a[j + 13] + a[j + 15];
        x3r = a[j + 12] - a[j + 14];
        x3i = a[j + 13] - a[j + 15];
        a[j + 8] = x0r + x2r;
        a[j + 9] = x0i + x2i;
        x0r -= x2r;
        x0i -= x2i;
        a[j + 12] = -wk2i * x0r - wk2r * x0i;
        a[j + 13] = -wk2i * x0i + wk2r * x0r;
        x0r = x1r - x3i;
        x0i = x1i + x3r;
        a[j + 10] = wk1r * x0r - wk1i * x0i;
        a[j + 11] = wk1r * x0i + wk1i * x0r;
        x0r = x1r + x3i;
        x0i = x1i - x3r;
        a[j + 14] = wk3r * x0r - wk3i * x0i;
        a[j + 15] = wk3r * x0i + wk3i * x0r;
      }
    }


    void cftmdl(int n, int l, double* a, double* w) {
      int j, j1, j2, j3, k, k1, k2, m, m2;
      double wk1r, wk1i, wk2r, wk2i, wk3r, wk3i;
      double x0r, x0i, x1r, x1i, x2r, x2i, x3r, x3i;

      m = l << 2;
      for (j = 0; j < l; j += 2) {
        j1 = j + l;
        j2 = j1 + l;
        j3 = j2 + l;
        x0r = a[j] + a[j1];
        x0i = a[j + 1] + a[j1 + 1];
        x1r = a[j] - a[j1];
        x1i = a[j + 1] - a[j1 + 1];
        x2r = a[j2] + a[j3];
        x2i = a[j2 + 1] + a[j3 + 1];
        x3r = a[j2] - a[j3];
        x3i = a[j2 + 1] - a[j3 + 1];
        a[j] = x0r + x2r;
        a[j + 1] = x0i + x2i;
        a[j2] = x0r - x2r;
        a[j2 + 1] = x0i - x2i;
        a[j1] = x1r - x3i;
        a[j1 + 1] = x1i + x3r;
        a[j3] = x1r + x3i;
        a[j3 + 1] = x1i - x3r;
      }
      wk1r = w[2];
      for (j = m; j < l + m; j += 2) {
        j1 = j + l;
        j2 = j1 + l;
        j3 = j2 + l;
        x0r = a[j] + a[j1];
        x0i = a[j + 1] + a[j1 + 1];
        x1r = a[j] - a[j1];
        x1i = a[j + 1] - a[j1 + 1];
        x2r = a[j2] + a[j3];
        x2i = a[j2 + 1] + a[j3 + 1];
        x3r = a[j2] - a[j3];
        x3i = a[j2 + 1] - a[j3 + 1];
        a[j] = x0r + x2r;
        a[j + 1] = x0i + x2i;
        a[j2] = x2i - x0i;
        a[j2 + 1] = x0r - x2r;
        x0r = x1r - x3i;
        x0i = x1i + x3r;
        a[j1] = wk1r * (x0r - x0i);
        a[j1 + 1] = wk1r * (x0r + x0i);
        x0r = x3i + x1r;
        x0i = x3r - x1i;
        a[j3] = wk1r * (x0i - x0r);
        a[j3 + 1] = wk1r * (x0i + x0r);
      }
      k1 = 0;
      m2 = 2 * m;
      for (k = m2; k < n; k += m2) {
        k1 += 2;
        k2 = 2 * k1;
        wk2r = w[k1];
        wk2i = w[k1 + 1];
        wk1r = w[k2];
        wk1i = w[k2 + 1];
        wk3r = wk1r - 2 * wk2i * wk1i;
        wk3i = 2 * wk2i * wk1r - wk1i;
        for (j = k; j < l + k; j += 2) {
          j1 = j + l;
          j2 = j1 + l;
          j3 = j2 + l;
          x0r = a[j] + a[j1];
          x0i = a[j + 1] + a[j1 + 1];
          x1r = a[j] - a[j1];
          x1i = a[j + 1] - a[j1 + 1];
          x2r = a[j2] + a[j3];
          x2i = a[j2 + 1] + a[j3 + 1];
          x3r = a[j2] - a[j3];
          x3i = a[j2 + 1] - a[j3 + 1];
          a[j] = x0r + x2r;
          a[j + 1] = x0i + x2i;
          x0r -= x2r;
          x0i -= x2i;
          a[j2] = wk2r * x0r - wk2i * x0i;
          a[j2 + 1] = wk2r * x0i + wk2i * x0r;
          x0r = x1r - x3i;
          x0i = x1i + x3r;
          a[j1] = wk1r * x0r - wk1i * x0i;
          a[j1 + 1] = wk1r * x0i + wk1i * x0r;
          x0r = x1r + x3i;
          x0i = x1i - x3r;
          a[j3] = wk3r * x0r - wk3i * x0i;
          a[j3 + 1] = wk3r * x0i + wk3i * x0r;
        }
        wk1r = w[k2 + 2];
        wk1i = w[k2 + 3];
        wk3r = wk1r - 2 * wk2r * wk1i;
        wk3i = 2 * wk2r * wk1r - wk1i;
        for (j = k + m; j < l + (k + m); j += 2) {
          j1 = j + l;
          j2 = j1 + l;
          j3 = j2 + l;
          x0r = a[j] + a[j1];
          x0i = a[j + 1] + a[j1 + 1];
          x1r = a[j] - a[j1];
          x1i = a[j + 1] - a[j1 + 1];
          x2r = a[j2] + a[j3];
          x2i = a[j2 + 1] + a[j3 + 1];
          x3r = a[j2] - a[j3];
          x3i = a[j2 + 1] - a[j3 + 1];
          a[j] = x0r + x2r;
          a[j + 1] = x0i + x2i;
          x0r -= x2r;
          x0i -= x2i;
          a[j2] = -wk2i * x0r - wk2r * x0i;
          a[j2 + 1] = -wk2i * x0i + wk2r * x0r;
          x0r = x1r - x3i;
          x0i = x1i + x3r;
          a[j1] = wk1r * x0r - wk1i * x0i;
          a[j1 + 1] = wk1r * x0i + wk1i * x0r;
          x0r = x1r + x3i;
          x0i = x1i - x3r;
          a[j3] = wk3r * x0r - wk3i * x0i;
          a[j3 + 1] = wk3r * x0i + wk3i * x0r;
        }
      }
    }


    void rftfsub(int n, double* a, int nc, double* c) {
      int j, k, kk, ks, m;
      double wkr, wki, xr, xi, yr, yi;

      m = n >> 1;
      ks = 2 * nc / m;
      kk = 0;
      for (j = 2; j < m; j += 2) {
        k = n - j;
        kk += ks;
        wkr = 0.5 - c[nc - kk];
        wki = c[kk];
        xr = a[j] - a[k];
        xi = a[j + 1] + a[k + 1];
        yr = wkr * xr - wki * xi;
        yi = wkr * xi + wki * xr;
        a[j] -= yr;
        a[j + 1] -= yi;
        a[k] += yr;
        a[k + 1] -= yi;
      }
    }


    void rftbsub(int n, double* a, int nc, double* c) {
      int j, k, kk, ks, m;
      double wkr, wki, xr, xi, yr, yi;

      a[1] = -a[1];
      m = n >> 1;
      ks = 2 * nc / m;
      kk = 0;
      for (j = 2; j < m; j += 2) {
        k = n - j;
        kk += ks;
        wkr = 0.5 - c[nc - kk];
        wki = c[kk];
        xr = a[j] - a[k];
        xi = a[j + 1] + a[k + 1];
        yr = wkr * xr + wki * xi;
        yi = wkr * xi - wki * xr;
        a[j] -= yr;
        a[j + 1] = yi - a[j + 1];
        a[k] += yr;
        a[k + 1] = yi - a[k + 1];
      }
      a[m + 1] = -a[m + 1];
    }
  };


  /**
   * @internal
   * @brief Concrete FFT implementation
   */
  typedef OouraFFT AudioFFTImplementation;
#endif // AUDIOFFT_OOURA_USED

#ifdef AUDIOFFT_APPLE_ACCELERATE_USED
  /**
   * @internal
   * @class AppleAccelerateFFT
   * @brief FFT implementation using the Apple Accelerate framework internally
   */
  class AppleAccelerateFFT : public detail::AudioFFTImpl {
  public:
    AppleAccelerateFFT() : detail::AudioFFTImpl(), _size(0), _powerOf2(0), _fftSetup(0), _re(), _im() {}

    AppleAccelerateFFT(const AppleAccelerateFFT&) = delete;
    AppleAccelerateFFT& operator=(const AppleAccelerateFFT&) = delete;

    virtual ~AppleAccelerateFFT() {
      init(0);
    }

    void init(size_t size) override {
      if (_fftSetup) {
        vDSP_destroy_fftsetup(_fftSetup);
        _size = 0;
        _powerOf2 = 0;
        _fftSetup = 0;
        _re.clear();
        _im.clear();
      }

      if (size > 0) {
        _size = size;
        _powerOf2 = 0;
        while ((1 << _powerOf2) < _size) {
          ++_powerOf2;
        }
        _fftSetup = vDSP_create_fftsetup(_powerOf2, FFT_RADIX2);
        _re.resize(_size / 2);
        _im.resize(_size / 2);
      }
    }

    void fft(const fftconvolver::Sample* data, fftconvolver::Sample* re, fftconvolver::Sample* im) override {
      const size_t size2 = _size / 2;
      DSPSplitComplex splitComplex;
      splitComplex.realp = re;
      splitComplex.imagp = im;
      vDSP_ctoz(reinterpret_cast<const COMPLEX*>(data), 2, &splitComplex, 1, size2);
      vDSP_fft_zrip(_fftSetup, &splitComplex, 1, _powerOf2, FFT_FORWARD);
      const fftconvolver::Sample factor = 0.5f;
      vDSP_vsmul(re, 1, &factor, re, 1, size2);
      vDSP_vsmul(im, 1, &factor, im, 1, size2);
      re[size2] = im[0];
      im[0] = 0.0f;
      im[size2] = 0.0f;
    }

    void ifft(fftconvolver::Sample* data, const fftconvolver::Sample* re, const fftconvolver::Sample* im) override {
      const size_t size2 = _size / 2;
      ::memcpy(_re.data(), re, size2 * sizeof(fftconvolver::Sample));
      ::memcpy(_im.data(), im, size2 * sizeof(fftconvolver::Sample));
      _im[0] = re[size2];
      DSPSplitComplex splitComplex;
      splitComplex.realp = _re.data();
      splitComplex.imagp = _im.data();
      vDSP_fft_zrip(_fftSetup, &splitComplex, 1, _powerOf2, FFT_INVERSE);
      vDSP_ztoc(&splitComplex, 1, reinterpret_cast<COMPLEX*>(data), 2, size2);
      const fftconvolver::Sample factor = 1.0f / static_cast<fftconvolver::Sample>(_size);
      vDSP_vsmul(data, 1, &factor, data, 1, _size);
    }

  private:
    size_t _size;
    size_t _powerOf2;
    FFTSetup _fftSetup;
    std::vector<fftconvolver::Sample> _re;
    std::vector<fftconvolver::Sample> _im;
  };

  /**
   * @internal
   * @brief Concrete FFT implementation
   */
  typedef AppleAccelerateFFT AudioFFTImplementation;
#endif // AUDIOFFT_APPLE_ACCELERATE_USED

  // ================================================================


  /**
   * @class AudioFFT
   * @brief Performs 1D FFTs
   */
  class AudioFFT {
  public:
    /**
     * @brief Constructor
     */
    AudioFFT() : _impl(new AudioFFTImplementation()) {}

    AudioFFT(const AudioFFT&) = delete;
    AudioFFT& operator=(const AudioFFT&) = delete;

    /**
     * @brief Destructor
     */
    ~AudioFFT() {}

    /**
     * @brief Initializes the FFT object
     * @param size Size of the real input (must be power 2)
     */
    void init(size_t size) {
      assert(detail::IsPowerOf2(size));
      _impl->init(size);
    }

    /**
     * @brief Performs the forward FFT
     * @param data The real input data (has to be of the length as specified in init())
     * @param re The real part of the complex output (has to be of length as returned by ComplexSize())
     * @param im The imaginary part of the complex output (has to be of length as returned by ComplexSize())
     */
    void fft(const fftconvolver::Sample* data, fftconvolver::Sample* re, fftconvolver::Sample* im) {
      _impl->fft(data, re, im);
    }

    /**
     * @brief Performs the inverse FFT
     * @param data The real output data (has to be of the length as specified in init())
     * @param re The real part of the complex input (has to be of length as returned by ComplexSize())
     * @param im The imaginary part of the complex input (has to be of length as returned by ComplexSize())
     */
    void ifft(fftconvolver::Sample* data, const fftconvolver::Sample* re, const fftconvolver::Sample* im) {
      _impl->ifft(data, re, im);
    }

    /**
     * @brief Calculates the necessary size of the real/imaginary complex arrays
     * @param size The size of the real data
     * @return The size of the real/imaginary complex arrays
     */
    static size_t ComplexSize(size_t size) {
      return (size / 2) + 1;
    }

  private:
    std::unique_ptr<detail::AudioFFTImpl> _impl;
  };

  /**
   * @deprecated
   * @brief Let's keep an AudioFFTBase type around for now because it has been here already in the 1st version in order to avoid breaking existing code.
   */
  typedef AudioFFT AudioFFTBase;

} // End of namespace

#endif // Header guard
