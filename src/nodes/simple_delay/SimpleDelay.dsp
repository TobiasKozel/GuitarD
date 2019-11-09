import("stdfaust.lib");
import("reverbs.lib");
import("misceffects.lib");


scaleFreq = exp(log(20) + _ * (log(20000) - log(20))), 1 : max : si.smooth(0.999);
linear2db(x) = 20*log10(x);// Max delay in seconds
maxDelay = 1;
time = vslider( "Time", 0.3, 0, maxDelay, 0.001) * ma.SR, 1 : max : si.smooth(0.999);
dry = ba.db2linear(vslider("Dry", 0, -60, 0, 0.1));
wet = ba.db2linear(vslider( "Wet", -8, -60, 0, 0.1));
decay = vslider( "Decay", 0.4, 0, 1, 0.01);

lowpass = scaleFreq(vslider( "Lowpass", 0.7, 0, 1, 0.001));
resonance = vslider( "Resonance", 1, 0.1, 2, 0.01);
pitch = vslider( "Pitch", 0, -1, 1, 0.01);
stereo = vslider( "Stereo", 0, 0, 1, 1);

delayLine = de.delay(ma.SR, time) : fi.resonlp(lowpass, resonance, decay) : ef.transpose(64, 32, pitch);
singleDelay = _<: _ * dry , (_ : delayLine + delayLine ~ _) * wet :> _;
process = _, _ <: (_, _ :> + : singleDelay <: _, _), singleDelay, singleDelay : ba.select2stereo(stereo);