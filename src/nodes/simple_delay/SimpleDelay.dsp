import("stdfaust.lib");
import("reverbs.lib");
import("misceffects.lib");


linear2db(x) = 20*log10(x);
maxDelay = 1; // Max delay in seconds
time = vslider( "Time", 0.3, 0, maxDelay, 0.001) * ma.SR, 1 : max : si.smooth(0.999);
dry = ba.db2linear(vslider("Dry", 0, -60, 0, 0.1));
wet = ba.db2linear(vslider( "Wet", -8, -60, 0, 0.1));
decay = vslider( "Decay", 0.4, 0, 1, 0.01);

lowpass = vslider( "Lowpass", 2000, 20, 20000, 1) : si.smooth(0.999);
resonance = vslider( "Resonance", 1, 0.1, 2, 0.01);
pitch = vslider( "Pitch", 0, -1, 1, 0.01);

delayLine = de.delay(ma.SR, time) : fi.resonlp(lowpass, resonance, decay) : ef.transpose(64, 32, pitch);
singleDelay = _<: _ * dry , (_ : delayLine + delayLine ~ _) * wet :> _;
process = singleDelay, singleDelay;