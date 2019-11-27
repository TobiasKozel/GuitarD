#pragma once
#include "src/node/Node.h"
 
//MIT License
//
//Copyright(c) 2018 Chris Johnson
//
//Permission is hereby granted, free of charge, to any person obtaining a copy
//of this softwareand associated documentation files(the "Software"), to deal
//in the Software without restriction, including without limitation the rights
//to use, copy, modify, merge, publish, distribute, sublicense, and /or sell
//copies of the Software, and to permit persons to whom the Software is
//furnished to do so, subject to the following conditions :
//
//The above copyright noticeand this permission notice shall be included in all
//copies or substantial portions of the Software.
//
//THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
//IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
//FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT.IN NO EVENT SHALL THE
//AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
//LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
//OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
//SOFTWARE.

class PowerSagNode final : public Node {
  double mIntensity = 0;
  double mDepth = 0.3;

  long double fpNShapeL;
  long double fpNShapeR;
  //default stuff
  double dL[9000];
  double dR[9000];
  double controlL;
  double controlR;
  int gcount;
public:
  PowerSagNode(const std::string pType) {
    for (int count = 0; count < 8999; count++) { dL[count] = 0; dR[count] = 0; }
    controlL = 0;
    controlR = 0;
    gcount = 0;
    fpNShapeL = 0.0;
    fpNShapeR = 0.0;
    mType = pType;
    shared.width = 200;
    shared.height = 100;
    addByPassParam();

    ParameterCoupling* p = new ParameterCoupling(
      "Depth", &mIntensity, 0.0, 0, 1.0, 0.01
    );
    p->x = -50;
    shared.parameters[shared.parameterCount] = p;
    shared.parameterCount++;

    p = new ParameterCoupling(
      "Speed", &mDepth, 0.3, 0.0, 1.0, 0.01
    );
    p->x = 50;
    shared.parameters[shared.parameterCount] = p;
    shared.parameterCount++;
  }

  void ProcessBlock(int nFrames) override {
    if (!inputsReady() || mIsProcessed || byPass()) { return; }
    sample** buffer = shared.socketsIn[0]->mConnectedTo->mParentBuffer;
    sample* in1 = buffer[0];
    sample* in2 = buffer[1];
    sample* out1 = mBuffersOut[0][0];
    sample* out2 = mBuffersOut[0][1];

    shared.parameters[1]->update();
    shared.parameters[2]->update();

    const double intensity = pow(mIntensity, 5) * 80.0;
    const double depthA = pow(mDepth, 2);
    int offsetA = (int)(depthA * 3900) + 1;


    while (--nFrames >= 0)
    {
      long double inputSampleL = *in1;
      long double inputSampleR = *in2;
      if (inputSampleL < 1.2e-38 && -inputSampleL < 1.2e-38) {
        static int noisesource = 0;
        //this declares a variable before anything else is compiled. It won't keep assigning
        //it to 0 for every sample, it's as if the declaration doesn't exist in this context,
        //but it lets me add this denormalization fix in a single place rather than updating
        //it in three different locations. The variable isn't thread-safe but this is only
        //a random seed and we can share it with whatever.
        noisesource = noisesource % 1700021; noisesource++;
        int residue = noisesource * noisesource;
        residue = residue % 170003; residue *= residue;
        residue = residue % 17011; residue *= residue;
        residue = residue % 1709; residue *= residue;
        residue = residue % 173; residue *= residue;
        residue = residue % 17;
        double applyresidue = residue;
        applyresidue *= 0.00000001;
        applyresidue *= 0.00000001;
        inputSampleL = applyresidue;
      }
      if (inputSampleR < 1.2e-38 && -inputSampleR < 1.2e-38) {
        static int noisesource = 0;
        noisesource = noisesource % 1700021; noisesource++;
        int residue = noisesource * noisesource;
        residue = residue % 170003; residue *= residue;
        residue = residue % 17011; residue *= residue;
        residue = residue % 1709; residue *= residue;
        residue = residue % 173; residue *= residue;
        residue = residue % 17;
        double applyresidue = residue;
        applyresidue *= 0.00000001;
        applyresidue *= 0.00000001;
        inputSampleR = applyresidue;
        //this denormalization routine produces a white noise at -300 dB which the noise
        //shaping will interact with to produce a bipolar output, but the noise is actually
        //all positive. That should stop any variables from going denormal, and the routine
        //only kicks in if digital black is input. As a final touch, if you save to 24-bit
        //the silence will return to being digital black again.
      }

      if (gcount < 0 || gcount > 4000) { gcount = 4000; }

      //doing L
      dL[gcount + 4000] = dL[gcount] = fabs(inputSampleL) * intensity;
      controlL += (dL[gcount] / offsetA);
      controlL -= (dL[gcount + offsetA] / offsetA);
      controlL -= 0.000001;
      double clamp = 1;
      if (controlL < 0) { controlL = 0; }
      if (controlL > 1) { clamp -= (controlL - 1); controlL = 1; }
      if (clamp < 0.5) { clamp = 0.5; }
      double thickness = ((1.0 - controlL) * 2.0) - 1.0;
      double out = fabs(thickness);
      double bridgerectifier = fabs(inputSampleL);
      if (bridgerectifier > 1.57079633) bridgerectifier = 1.57079633;
      //max value for sine function
      if (thickness > 0) bridgerectifier = sin(bridgerectifier);
      else bridgerectifier = 1 - cos(bridgerectifier);
      //produce either boosted or starved version
      if (inputSampleL > 0) inputSampleL = (inputSampleL * (1 - out)) + (bridgerectifier * out);
      else inputSampleL = (inputSampleL * (1 - out)) - (bridgerectifier * out);
      //blend according to density control
      inputSampleL *= clamp;
      //end L

      //doing R
      dR[gcount + 4000] = dR[gcount] = fabs(inputSampleR) * intensity;
      controlR += (dR[gcount] / offsetA);
      controlR -= (dR[gcount + offsetA] / offsetA);
      controlR -= 0.000001;
      clamp = 1;
      if (controlR < 0) { controlR = 0; }
      if (controlR > 1) { clamp -= (controlR - 1); controlR = 1; }
      if (clamp < 0.5) { clamp = 0.5; }
      thickness = ((1.0 - controlR) * 2.0) - 1.0;
      out = fabs(thickness);
      bridgerectifier = fabs(inputSampleR);
      if (bridgerectifier > 1.57079633) bridgerectifier = 1.57079633;
      //max value for sine function
      if (thickness > 0) bridgerectifier = sin(bridgerectifier);
      else bridgerectifier = 1 - cos(bridgerectifier);
      //produce either boosted or starved version
      if (inputSampleR > 0) inputSampleR = (inputSampleR * (1 - out)) + (bridgerectifier * out);
      else inputSampleR = (inputSampleR * (1 - out)) - (bridgerectifier * out);
      //blend according to density control
      inputSampleR *= clamp;
      //end R

      gcount--;


      //stereo 64 bit dither, made small and tidy.
      int expon; frexp((double)inputSampleL, &expon);
      long double dither = (rand() / (RAND_MAX * 7.737125245533627e+25)) * pow(2, expon + 62);
      dither /= 536870912.0; //needs this to scale to 64 bit zone
      inputSampleL += (dither - fpNShapeL); fpNShapeL = dither;
      frexp((double)inputSampleR, &expon);
      dither = (rand() / (RAND_MAX * 7.737125245533627e+25)) * pow(2, expon + 62);
      dither /= 536870912.0; //needs this to scale to 64 bit zone
      inputSampleR += (dither - fpNShapeR); fpNShapeR = dither;
      //end 64 bit dither

      *out1 = inputSampleL;
      *out2 = inputSampleR;

      *in1++;
      *in2++;
      *out1++;
      *out2++;
    }

    mIsProcessed = true;
  }
};

