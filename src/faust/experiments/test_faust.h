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
#include <math.h>

static double TestDsp_faustpower2_f(double value) {
	return (value * value);
	
}

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
	double fConst0;
	double fConst1;
	double fConst2;
	double fConst3;
	double fConst4;
	double fConst5;
	double fConst6;
	double fConst7;
	double fConst8;
	double fConst9;
	double fConst10;
	double fRec10[2];
	int IOTA;
	double fVec0[32768];
	double fConst11;
	int iConst12;
	double fVec1[2048];
	int iConst13;
	double fRec8[2];
	double fConst14;
	double fConst15;
	double fConst16;
	double fConst17;
	double fConst18;
	double fConst19;
	double fConst20;
	double fConst21;
	double fConst22;
	double fRec13[2];
	double fVec2[32768];
	double fConst23;
	int iConst24;
	double fVec3[4096];
	int iConst25;
	double fRec11[2];
	double fConst26;
	double fConst27;
	double fConst28;
	double fConst29;
	double fConst30;
	double fConst31;
	double fConst32;
	double fConst33;
	double fConst34;
	double fRec16[2];
	double fVec4[16384];
	double fConst35;
	int iConst36;
	double fVec5[4096];
	int iConst37;
	double fRec14[2];
	double fConst38;
	double fConst39;
	double fConst40;
	double fConst41;
	double fConst42;
	double fConst43;
	double fConst44;
	double fConst45;
	double fConst46;
	double fRec19[2];
	double fVec6[32768];
	double fConst47;
	int iConst48;
	double fVec7[4096];
	int iConst49;
	double fRec17[2];
	double fConst50;
	double fConst51;
	double fConst52;
	double fConst53;
	double fConst54;
	double fConst55;
	double fConst56;
	double fConst57;
	double fConst58;
	double fRec22[2];
	double fVec8[16384];
	double fConst59;
	int iConst60;
	double fVec9[2048];
	int iConst61;
	double fRec20[2];
	double fConst62;
	double fConst63;
	double fConst64;
	double fConst65;
	double fConst66;
	double fConst67;
	double fConst68;
	double fConst69;
	double fConst70;
	double fRec25[2];
	double fVec10[16384];
	double fConst71;
	int iConst72;
	double fVec11[4096];
	int iConst73;
	double fRec23[2];
	double fConst74;
	double fConst75;
	double fConst76;
	double fConst77;
	double fConst78;
	double fConst79;
	double fConst80;
	double fConst81;
	double fConst82;
	double fRec28[2];
	double fVec12[16384];
	double fConst83;
	int iConst84;
	double fVec13[4096];
	int iConst85;
	double fRec26[2];
	double fConst86;
	double fConst87;
	double fConst88;
	double fConst89;
	double fConst90;
	double fConst91;
	double fConst92;
	double fConst93;
	double fConst94;
	double fRec31[2];
	double fVec14[16384];
	double fConst95;
	int iConst96;
	double fVec15[2048];
	int iConst97;
	double fRec29[2];
	double fRec0[2];
	double fRec1[2];
	double fRec2[2];
	double fRec3[2];
	double fRec4[2];
	double fRec5[2];
	double fRec6[2];
	double fRec7[2];
	
 public:
	
	void metadata(Meta* m) { 
		m->declare("basics.lib/name", "Faust Basic Element Library");
		m->declare("basics.lib/version", "0.0");
		m->declare("delays.lib/name", "Faust Delay Library");
		m->declare("delays.lib/version", "0.0");
		m->declare("filename", "test");
		m->declare("filters.lib/name", "Faust Filters Library");
		m->declare("filters.lib/version", "0.0");
		m->declare("maths.lib/author", "GRAME");
		m->declare("maths.lib/copyright", "GRAME");
		m->declare("maths.lib/license", "LGPL with exception");
		m->declare("maths.lib/name", "Faust Math Library");
		m->declare("maths.lib/version", "2.1");
		m->declare("name", "test");
		m->declare("reverbs.lib/name", "Faust Reverb Library");
		m->declare("reverbs.lib/version", "0.0");
		m->declare("routes.lib/name", "Faust Signal Routing Library");
		m->declare("routes.lib/version", "0.0");
		m->declare("signals.lib/name", "Faust Signal Routing Library");
		m->declare("signals.lib/version", "0.0");
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
		fConst0 = std::min<double>(192000.0, std::max<double>(1.0, double(fSamplingFreq)));
		fConst1 = std::floor(((0.21999099999999999 * fConst0) + 0.5));
		fConst2 = std::exp((0.10000000000000001 * ((0.0 - (6.9077552789821377 * fConst1)) / fConst0)));
		fConst3 = TestDsp_faustpower2_f(fConst2);
		fConst4 = std::cos((25132.741228718343 / fConst0));
		fConst5 = (1.0 - (fConst3 * fConst4));
		fConst6 = (1.0 - fConst3);
		fConst7 = (fConst5 / fConst6);
		fConst8 = std::sqrt(std::max<double>(0.0, ((TestDsp_faustpower2_f(fConst5) / TestDsp_faustpower2_f(fConst6)) + -1.0)));
		fConst9 = (fConst7 - fConst8);
		fConst10 = (fConst2 * (fConst8 + (1.0 - fConst7)));
		fConst11 = std::floor(((0.019123000000000001 * fConst0) + 0.5));
		iConst12 = int(std::min<double>(16384.0, std::max<double>(0.0, (fConst1 - fConst11))));
		iConst13 = int(std::min<double>(1024.0, std::max<double>(0.0, (fConst11 + -1.0))));
		fConst14 = std::floor(((0.25689099999999998 * fConst0) + 0.5));
		fConst15 = std::exp((0.10000000000000001 * ((0.0 - (6.9077552789821377 * fConst14)) / fConst0)));
		fConst16 = TestDsp_faustpower2_f(fConst15);
		fConst17 = (1.0 - (fConst16 * fConst4));
		fConst18 = (1.0 - fConst16);
		fConst19 = (fConst17 / fConst18);
		fConst20 = std::sqrt(std::max<double>(0.0, ((TestDsp_faustpower2_f(fConst17) / TestDsp_faustpower2_f(fConst18)) + -1.0)));
		fConst21 = (fConst19 - fConst20);
		fConst22 = (fConst15 * (fConst20 + (1.0 - fConst19)));
		fConst23 = std::floor(((0.027333 * fConst0) + 0.5));
		iConst24 = int(std::min<double>(16384.0, std::max<double>(0.0, (fConst14 - fConst23))));
		iConst25 = int(std::min<double>(2048.0, std::max<double>(0.0, (fConst23 + -1.0))));
		fConst26 = std::floor(((0.192303 * fConst0) + 0.5));
		fConst27 = std::exp((0.10000000000000001 * ((0.0 - (6.9077552789821377 * fConst26)) / fConst0)));
		fConst28 = TestDsp_faustpower2_f(fConst27);
		fConst29 = (1.0 - (fConst28 * fConst4));
		fConst30 = (1.0 - fConst28);
		fConst31 = (fConst29 / fConst30);
		fConst32 = std::sqrt(std::max<double>(0.0, ((TestDsp_faustpower2_f(fConst29) / TestDsp_faustpower2_f(fConst30)) + -1.0)));
		fConst33 = (fConst31 - fConst32);
		fConst34 = (fConst27 * (fConst32 + (1.0 - fConst31)));
		fConst35 = std::floor(((0.029291000000000001 * fConst0) + 0.5));
		iConst36 = int(std::min<double>(8192.0, std::max<double>(0.0, (fConst26 - fConst35))));
		iConst37 = int(std::min<double>(2048.0, std::max<double>(0.0, (fConst35 + -1.0))));
		fConst38 = std::floor(((0.21038899999999999 * fConst0) + 0.5));
		fConst39 = std::exp((0.10000000000000001 * ((0.0 - (6.9077552789821377 * fConst38)) / fConst0)));
		fConst40 = TestDsp_faustpower2_f(fConst39);
		fConst41 = (1.0 - (fConst40 * fConst4));
		fConst42 = (1.0 - fConst40);
		fConst43 = (fConst41 / fConst42);
		fConst44 = std::sqrt(std::max<double>(0.0, ((TestDsp_faustpower2_f(fConst41) / TestDsp_faustpower2_f(fConst42)) + -1.0)));
		fConst45 = (fConst43 - fConst44);
		fConst46 = (fConst39 * (fConst44 + (1.0 - fConst43)));
		fConst47 = std::floor(((0.024421000000000002 * fConst0) + 0.5));
		iConst48 = int(std::min<double>(16384.0, std::max<double>(0.0, (fConst38 - fConst47))));
		iConst49 = int(std::min<double>(2048.0, std::max<double>(0.0, (fConst47 + -1.0))));
		fConst50 = std::floor(((0.125 * fConst0) + 0.5));
		fConst51 = std::exp((0.10000000000000001 * ((0.0 - (6.9077552789821377 * fConst50)) / fConst0)));
		fConst52 = TestDsp_faustpower2_f(fConst51);
		fConst53 = (1.0 - (fConst52 * fConst4));
		fConst54 = (1.0 - fConst52);
		fConst55 = (fConst53 / fConst54);
		fConst56 = std::sqrt(std::max<double>(0.0, ((TestDsp_faustpower2_f(fConst53) / TestDsp_faustpower2_f(fConst54)) + -1.0)));
		fConst57 = (fConst55 - fConst56);
		fConst58 = (fConst51 * (fConst56 + (1.0 - fConst55)));
		fConst59 = std::floor(((0.013457999999999999 * fConst0) + 0.5));
		iConst60 = int(std::min<double>(8192.0, std::max<double>(0.0, (fConst50 - fConst59))));
		iConst61 = int(std::min<double>(1024.0, std::max<double>(0.0, (fConst59 + -1.0))));
		fConst62 = std::floor(((0.12783700000000001 * fConst0) + 0.5));
		fConst63 = std::exp((0.10000000000000001 * ((0.0 - (6.9077552789821377 * fConst62)) / fConst0)));
		fConst64 = TestDsp_faustpower2_f(fConst63);
		fConst65 = (1.0 - (fConst64 * fConst4));
		fConst66 = (1.0 - fConst64);
		fConst67 = std::sqrt(std::max<double>(0.0, ((TestDsp_faustpower2_f(fConst65) / TestDsp_faustpower2_f(fConst66)) + -1.0)));
		fConst68 = (fConst65 / fConst66);
		fConst69 = (fConst63 * (fConst67 + (1.0 - fConst68)));
		fConst70 = (fConst68 - fConst67);
		fConst71 = std::floor(((0.031604 * fConst0) + 0.5));
		iConst72 = int(std::min<double>(8192.0, std::max<double>(0.0, (fConst62 - fConst71))));
		iConst73 = int(std::min<double>(2048.0, std::max<double>(0.0, (fConst71 + -1.0))));
		fConst74 = std::floor(((0.17471300000000001 * fConst0) + 0.5));
		fConst75 = std::exp((0.10000000000000001 * ((0.0 - (6.9077552789821377 * fConst74)) / fConst0)));
		fConst76 = TestDsp_faustpower2_f(fConst75);
		fConst77 = (1.0 - (fConst76 * fConst4));
		fConst78 = (1.0 - fConst76);
		fConst79 = std::sqrt(std::max<double>(0.0, ((TestDsp_faustpower2_f(fConst77) / TestDsp_faustpower2_f(fConst78)) + -1.0)));
		fConst80 = (fConst77 / fConst78);
		fConst81 = (fConst75 * (fConst79 + (1.0 - fConst80)));
		fConst82 = (fConst80 - fConst79);
		fConst83 = std::floor(((0.022904000000000001 * fConst0) + 0.5));
		iConst84 = int(std::min<double>(8192.0, std::max<double>(0.0, (fConst74 - fConst83))));
		iConst85 = int(std::min<double>(2048.0, std::max<double>(0.0, (fConst83 + -1.0))));
		fConst86 = std::floor(((0.15312899999999999 * fConst0) + 0.5));
		fConst87 = std::exp((0.10000000000000001 * ((0.0 - (6.9077552789821377 * fConst86)) / fConst0)));
		fConst88 = TestDsp_faustpower2_f(fConst87);
		fConst89 = (1.0 - (fConst88 * fConst4));
		fConst90 = (1.0 - fConst88);
		fConst91 = (fConst89 / fConst90);
		fConst92 = std::sqrt(std::max<double>(0.0, ((TestDsp_faustpower2_f(fConst89) / TestDsp_faustpower2_f(fConst90)) + -1.0)));
		fConst93 = (fConst91 - fConst92);
		fConst94 = (fConst87 * (fConst92 + (1.0 - fConst91)));
		fConst95 = std::floor(((0.020346 * fConst0) + 0.5));
		iConst96 = int(std::min<double>(8192.0, std::max<double>(0.0, (fConst86 - fConst95))));
		iConst97 = int(std::min<double>(1024.0, std::max<double>(0.0, (fConst95 + -1.0))));
		
	}
	
	virtual void instanceResetUserInterface() {
		fHslider0 = FAUSTFLOAT(0.5);
		
	}
	
	virtual void instanceClear() {
		for (int l0 = 0; (l0 < 2); l0 = (l0 + 1)) {
			fRec10[l0] = 0.0;
			
		}
		IOTA = 0;
		for (int l1 = 0; (l1 < 32768); l1 = (l1 + 1)) {
			fVec0[l1] = 0.0;
			
		}
		for (int l2 = 0; (l2 < 2048); l2 = (l2 + 1)) {
			fVec1[l2] = 0.0;
			
		}
		for (int l3 = 0; (l3 < 2); l3 = (l3 + 1)) {
			fRec8[l3] = 0.0;
			
		}
		for (int l4 = 0; (l4 < 2); l4 = (l4 + 1)) {
			fRec13[l4] = 0.0;
			
		}
		for (int l5 = 0; (l5 < 32768); l5 = (l5 + 1)) {
			fVec2[l5] = 0.0;
			
		}
		for (int l6 = 0; (l6 < 4096); l6 = (l6 + 1)) {
			fVec3[l6] = 0.0;
			
		}
		for (int l7 = 0; (l7 < 2); l7 = (l7 + 1)) {
			fRec11[l7] = 0.0;
			
		}
		for (int l8 = 0; (l8 < 2); l8 = (l8 + 1)) {
			fRec16[l8] = 0.0;
			
		}
		for (int l9 = 0; (l9 < 16384); l9 = (l9 + 1)) {
			fVec4[l9] = 0.0;
			
		}
		for (int l10 = 0; (l10 < 4096); l10 = (l10 + 1)) {
			fVec5[l10] = 0.0;
			
		}
		for (int l11 = 0; (l11 < 2); l11 = (l11 + 1)) {
			fRec14[l11] = 0.0;
			
		}
		for (int l12 = 0; (l12 < 2); l12 = (l12 + 1)) {
			fRec19[l12] = 0.0;
			
		}
		for (int l13 = 0; (l13 < 32768); l13 = (l13 + 1)) {
			fVec6[l13] = 0.0;
			
		}
		for (int l14 = 0; (l14 < 4096); l14 = (l14 + 1)) {
			fVec7[l14] = 0.0;
			
		}
		for (int l15 = 0; (l15 < 2); l15 = (l15 + 1)) {
			fRec17[l15] = 0.0;
			
		}
		for (int l16 = 0; (l16 < 2); l16 = (l16 + 1)) {
			fRec22[l16] = 0.0;
			
		}
		for (int l17 = 0; (l17 < 16384); l17 = (l17 + 1)) {
			fVec8[l17] = 0.0;
			
		}
		for (int l18 = 0; (l18 < 2048); l18 = (l18 + 1)) {
			fVec9[l18] = 0.0;
			
		}
		for (int l19 = 0; (l19 < 2); l19 = (l19 + 1)) {
			fRec20[l19] = 0.0;
			
		}
		for (int l20 = 0; (l20 < 2); l20 = (l20 + 1)) {
			fRec25[l20] = 0.0;
			
		}
		for (int l21 = 0; (l21 < 16384); l21 = (l21 + 1)) {
			fVec10[l21] = 0.0;
			
		}
		for (int l22 = 0; (l22 < 4096); l22 = (l22 + 1)) {
			fVec11[l22] = 0.0;
			
		}
		for (int l23 = 0; (l23 < 2); l23 = (l23 + 1)) {
			fRec23[l23] = 0.0;
			
		}
		for (int l24 = 0; (l24 < 2); l24 = (l24 + 1)) {
			fRec28[l24] = 0.0;
			
		}
		for (int l25 = 0; (l25 < 16384); l25 = (l25 + 1)) {
			fVec12[l25] = 0.0;
			
		}
		for (int l26 = 0; (l26 < 4096); l26 = (l26 + 1)) {
			fVec13[l26] = 0.0;
			
		}
		for (int l27 = 0; (l27 < 2); l27 = (l27 + 1)) {
			fRec26[l27] = 0.0;
			
		}
		for (int l28 = 0; (l28 < 2); l28 = (l28 + 1)) {
			fRec31[l28] = 0.0;
			
		}
		for (int l29 = 0; (l29 < 16384); l29 = (l29 + 1)) {
			fVec14[l29] = 0.0;
			
		}
		for (int l30 = 0; (l30 < 2048); l30 = (l30 + 1)) {
			fVec15[l30] = 0.0;
			
		}
		for (int l31 = 0; (l31 < 2); l31 = (l31 + 1)) {
			fRec29[l31] = 0.0;
			
		}
		for (int l32 = 0; (l32 < 2); l32 = (l32 + 1)) {
			fRec0[l32] = 0.0;
			
		}
		for (int l33 = 0; (l33 < 2); l33 = (l33 + 1)) {
			fRec1[l33] = 0.0;
			
		}
		for (int l34 = 0; (l34 < 2); l34 = (l34 + 1)) {
			fRec2[l34] = 0.0;
			
		}
		for (int l35 = 0; (l35 < 2); l35 = (l35 + 1)) {
			fRec3[l35] = 0.0;
			
		}
		for (int l36 = 0; (l36 < 2); l36 = (l36 + 1)) {
			fRec4[l36] = 0.0;
			
		}
		for (int l37 = 0; (l37 < 2); l37 = (l37 + 1)) {
			fRec5[l37] = 0.0;
			
		}
		for (int l38 = 0; (l38 < 2); l38 = (l38 + 1)) {
			fRec6[l38] = 0.0;
			
		}
		for (int l39 = 0; (l39 < 2); l39 = (l39 + 1)) {
			fRec7[l39] = 0.0;
			
		}
		
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
		double fSlow0 = (0.37 * double(fHslider0));
		for (int i = 0; (i < count); i = (i + 1)) {
			fRec10[0] = ((fConst9 * fRec10[1]) + (fConst10 * fRec7[1]));
			fVec0[(IOTA & 32767)] = ((0.35355339059327373 * fRec10[0]) + 9.9999999999999995e-21);
			double fTemp0 = (0.29999999999999999 * double(input1[i]));
			double fTemp1 = (((0.59999999999999998 * fRec8[1]) + fVec0[((IOTA - iConst12) & 32767)]) - fTemp0);
			fVec1[(IOTA & 2047)] = fTemp1;
			fRec8[0] = fVec1[((IOTA - iConst13) & 2047)];
			double fRec9 = (0.0 - (0.59999999999999998 * fTemp1));
			fRec13[0] = ((fConst21 * fRec13[1]) + (fConst22 * fRec3[1]));
			fVec2[(IOTA & 32767)] = ((0.35355339059327373 * fRec13[0]) + 9.9999999999999995e-21);
			double fTemp2 = (((0.59999999999999998 * fRec11[1]) + fVec2[((IOTA - iConst24) & 32767)]) - fTemp0);
			fVec3[(IOTA & 4095)] = fTemp2;
			fRec11[0] = fVec3[((IOTA - iConst25) & 4095)];
			double fRec12 = (0.0 - (0.59999999999999998 * fTemp2));
			fRec16[0] = ((fConst33 * fRec16[1]) + (fConst34 * fRec5[1]));
			fVec4[(IOTA & 16383)] = ((0.35355339059327373 * fRec16[0]) + 9.9999999999999995e-21);
			double fTemp3 = ((fTemp0 + (0.59999999999999998 * fRec14[1])) + fVec4[((IOTA - iConst36) & 16383)]);
			fVec5[(IOTA & 4095)] = fTemp3;
			fRec14[0] = fVec5[((IOTA - iConst37) & 4095)];
			double fRec15 = (0.0 - (0.59999999999999998 * fTemp3));
			fRec19[0] = ((fConst45 * fRec19[1]) + (fConst46 * fRec1[1]));
			fVec6[(IOTA & 32767)] = ((0.35355339059327373 * fRec19[0]) + 9.9999999999999995e-21);
			double fTemp4 = (fVec6[((IOTA - iConst48) & 32767)] + ((0.59999999999999998 * fRec17[1]) + fTemp0));
			fVec7[(IOTA & 4095)] = fTemp4;
			fRec17[0] = fVec7[((IOTA - iConst49) & 4095)];
			double fRec18 = (0.0 - (0.59999999999999998 * fTemp4));
			fRec22[0] = ((fConst57 * fRec22[1]) + (fConst58 * fRec6[1]));
			fVec8[(IOTA & 16383)] = ((0.35355339059327373 * fRec22[0]) + 9.9999999999999995e-21);
			double fTemp5 = (0.29999999999999999 * double(input0[i]));
			double fTemp6 = (fVec8[((IOTA - iConst60) & 16383)] - (fTemp5 + (0.59999999999999998 * fRec20[1])));
			fVec9[(IOTA & 2047)] = fTemp6;
			fRec20[0] = fVec9[((IOTA - iConst61) & 2047)];
			double fRec21 = (0.59999999999999998 * fTemp6);
			fRec25[0] = ((fConst69 * fRec2[1]) + (fConst70 * fRec25[1]));
			fVec10[(IOTA & 16383)] = ((0.35355339059327373 * fRec25[0]) + 9.9999999999999995e-21);
			double fTemp7 = (fVec10[((IOTA - iConst72) & 16383)] - (fTemp5 + (0.59999999999999998 * fRec23[1])));
			fVec11[(IOTA & 4095)] = fTemp7;
			fRec23[0] = fVec11[((IOTA - iConst73) & 4095)];
			double fRec24 = (0.59999999999999998 * fTemp7);
			fRec28[0] = ((fConst81 * fRec4[1]) + (fConst82 * fRec28[1]));
			fVec12[(IOTA & 16383)] = ((0.35355339059327373 * fRec28[0]) + 9.9999999999999995e-21);
			double fTemp8 = ((fTemp5 + fVec12[((IOTA - iConst84) & 16383)]) - (0.59999999999999998 * fRec26[1]));
			fVec13[(IOTA & 4095)] = fTemp8;
			fRec26[0] = fVec13[((IOTA - iConst85) & 4095)];
			double fRec27 = (0.59999999999999998 * fTemp8);
			fRec31[0] = ((fConst93 * fRec31[1]) + (fConst94 * fRec0[1]));
			fVec14[(IOTA & 16383)] = ((0.35355339059327373 * fRec31[0]) + 9.9999999999999995e-21);
			double fTemp9 = ((fVec14[((IOTA - iConst96) & 16383)] + fTemp5) - (0.59999999999999998 * fRec29[1]));
			fVec15[(IOTA & 2047)] = fTemp9;
			fRec29[0] = fVec15[((IOTA - iConst97) & 2047)];
			double fRec30 = (0.59999999999999998 * fTemp9);
			double fTemp10 = (fRec30 + fRec27);
			double fTemp11 = (fRec21 + (fRec24 + fTemp10));
			fRec0[0] = (fRec8[1] + (fRec11[1] + (fRec14[1] + (fRec17[1] + (fRec20[1] + (fRec23[1] + (fRec26[1] + (fRec29[1] + (fRec9 + (fRec12 + (fRec15 + (fRec18 + fTemp11))))))))))));
			fRec1[0] = ((fRec20[1] + (fRec23[1] + (fRec26[1] + (fRec29[1] + fTemp11)))) - (fRec8[1] + (fRec11[1] + (fRec14[1] + (fRec17[1] + (fRec9 + (fRec12 + (fRec18 + fRec15))))))));
			double fTemp12 = (fRec24 + fRec21);
			fRec2[0] = ((fRec14[1] + (fRec17[1] + (fRec26[1] + (fRec29[1] + (fRec15 + (fRec18 + fTemp10)))))) - (fRec8[1] + (fRec11[1] + (fRec20[1] + (fRec23[1] + (fRec9 + (fRec12 + fTemp12)))))));
			fRec3[0] = ((fRec8[1] + (fRec11[1] + (fRec26[1] + (fRec29[1] + (fRec9 + (fRec12 + fTemp10)))))) - (fRec14[1] + (fRec17[1] + (fRec20[1] + (fRec23[1] + (fRec15 + (fRec18 + fTemp12)))))));
			double fTemp13 = (fRec30 + fRec24);
			double fTemp14 = (fRec27 + fRec21);
			fRec4[0] = ((fRec11[1] + (fRec17[1] + (fRec23[1] + (fRec29[1] + (fRec12 + (fRec18 + fTemp13)))))) - (fRec8[1] + (fRec14[1] + (fRec20[1] + (fRec26[1] + (fRec9 + (fRec15 + fTemp14)))))));
			fRec5[0] = ((fRec8[1] + (fRec14[1] + (fRec23[1] + (fRec29[1] + (fRec9 + (fRec15 + fTemp13)))))) - (fRec11[1] + (fRec17[1] + (fRec20[1] + (fRec26[1] + (fRec12 + (fRec18 + fTemp14)))))));
			double fTemp15 = (fRec30 + fRec21);
			double fTemp16 = (fRec27 + fRec24);
			fRec6[0] = ((fRec8[1] + (fRec17[1] + (fRec20[1] + (fRec29[1] + (fRec9 + (fRec18 + fTemp15)))))) - (fRec11[1] + (fRec14[1] + (fRec23[1] + (fRec26[1] + (fRec12 + (fRec15 + fTemp16)))))));
			fRec7[0] = ((fRec11[1] + (fRec14[1] + (fRec20[1] + (fRec29[1] + (fRec12 + (fRec15 + fTemp15)))))) - (fRec8[1] + (fRec17[1] + (fRec23[1] + (fRec26[1] + (fRec9 + (fRec18 + fTemp16)))))));
			output0[i] = FAUSTFLOAT((fSlow0 * (fRec2[0] + fRec1[0])));
			output1[i] = FAUSTFLOAT((fSlow0 * (fRec1[0] - fRec2[0])));
			fRec10[1] = fRec10[0];
			IOTA = (IOTA + 1);
			fRec8[1] = fRec8[0];
			fRec13[1] = fRec13[0];
			fRec11[1] = fRec11[0];
			fRec16[1] = fRec16[0];
			fRec14[1] = fRec14[0];
			fRec19[1] = fRec19[0];
			fRec17[1] = fRec17[0];
			fRec22[1] = fRec22[0];
			fRec20[1] = fRec20[0];
			fRec25[1] = fRec25[0];
			fRec23[1] = fRec23[0];
			fRec28[1] = fRec28[0];
			fRec26[1] = fRec26[0];
			fRec31[1] = fRec31[0];
			fRec29[1] = fRec29[0];
			fRec0[1] = fRec0[0];
			fRec1[1] = fRec1[0];
			fRec2[1] = fRec2[0];
			fRec3[1] = fRec3[0];
			fRec4[1] = fRec4[0];
			fRec5[1] = fRec5[0];
			fRec6[1] = fRec6[0];
			fRec7[1] = fRec7[0];
			
		}
		
	}

	
};

#endif
