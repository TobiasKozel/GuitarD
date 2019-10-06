import("reverbs.lib");
import("misceffects.lib");

maxDelay = 48000;
time = vslider( "Time", 7000, 0, maxDelay, 1);
dry = vslider( "Dry", 1, 0, 1, 0.01);
wet = vslider( "Wet", 1, 0, 1, 0.01);
decay = vslider( "Decay", 0.7, 0, 1, 0.01);
lowpass = vslider( "Lowpass", 9000, 0, 20000, 1);
resonance = vslider( "Resonance", 1, 0.1, 2, 0.01);
pitch = vslider( "Pitch", 0, -1, 1, 0.01);

delay = de.delay(ma.SR, time) : fi.resonlp(lowpass, resonance, decay) : ef.transpose(64, 32, pitch);
process = _, _ :> + <: _ * dry , (_ : delay + delay ~ _) * wet :> + <: _, _;