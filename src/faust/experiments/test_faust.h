/* ------------------------------------------------------------
name: "test"
Code generated with Faust 2.14.4 (https://faust.grame.fr)
Compilation options: -double -ftz 0
------------------------------------------------------------ */

#ifndef  __TestDsp_H__
#define  __TestDsp_H__

#include "src/faust/FaustHeadlessDsp.h"


#ifndef FAUSTFLOAT
#define FAUSTFLOAT float
#endif 

#include <algorithm>
#include <cmath>


#ifndef FAUSTCLASS 
#define FAUSTCLASS TestDsp
#endif
#ifdef __APPLE__ 
#define exp10f __exp10f
#define exp10 __exp10
#endif

class TestDsp : public FaustHeadlessDsp {
	
 private:
	
	FAUSTFLOAT fHslider0;
	int fSamplingFreq;
	
 public:
	
	void metadata(Meta* m) { 
		m->declare("filename", "test");
		m->declare("name", "test");
	}

	virtual int getNumInputs() {
		return 2;
		
	}
	virtual int getNumOutputs() {
		return 2;
		
	}
	virtual int getInputRate(int channel) {
		int rate;
		switch (channel) {
			case 0: {
				rate = 1;
				break;
			}
			case 1: {
				rate = 1;
				break;
			}
			default: {
				rate = -1;
				break;
			}
			
		}
		return rate;
		
	}
	virtual int getOutputRate(int channel) {
		int rate;
		switch (channel) {
			case 0: {
				rate = 1;
				break;
			}
			case 1: {
				rate = 1;
				break;
			}
			default: {
				rate = -1;
				break;
			}
			
		}
		return rate;
		
	}
	
	static void classInit(int samplingFreq) {
		
	}
	
	virtual void instanceConstants(int samplingFreq) {
		fSamplingFreq = samplingFreq;
		
	}
	
	virtual void instanceResetUserInterface() {
		fHslider0 = FAUSTFLOAT(0.5);
		
	}
	
	virtual void instanceClear() {
		
	}
	
	virtual void init(int samplingFreq) {
		classInit(samplingFreq);
		instanceInit(samplingFreq);
	}
	virtual void instanceInit(int samplingFreq) {
		instanceConstants(samplingFreq);
		instanceResetUserInterface();
		instanceClear();
	}
	
	virtual TestDsp* clone() {
		return new TestDsp();
	}
	virtual int getSampleRate() {
		return fSamplingFreq;
		
	}
	
	virtual void buildUserInterface(UI* ui_interface) {
		ui_interface->openVerticalBox("test");
		ui_interface->addHorizontalSlider("gain", &fHslider0, 0.5, 0.0, 1.0, 0.01);
		ui_interface->closeBox();
		
	}
	
	virtual void compute(int count, FAUSTFLOAT** inputs, FAUSTFLOAT** outputs) {
		FAUSTFLOAT* input0 = inputs[0];
		FAUSTFLOAT* input1 = inputs[1];
		FAUSTFLOAT* output0 = outputs[0];
		FAUSTFLOAT* output1 = outputs[1];
		double fSlow0 = double(fHslider0);
		for (int i = 0; (i < count); i = (i + 1)) {
			output0[i] = FAUSTFLOAT((fSlow0 * double(input0[i])));
			output1[i] = FAUSTFLOAT((fSlow0 * double(input1[i])));
			
		}
		
	}

	
};

#endif
