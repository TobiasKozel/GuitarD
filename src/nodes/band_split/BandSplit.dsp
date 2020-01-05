import("stdfaust.lib");

maxf = 20000;
minf = 20;

lowf = vslider("Low", 300, minf, maxf, 1) : si.smooth(0.999);
highf = (vslider("High", 2000, minf, maxf, 1), lowf: max) : si.smooth(0.999);

low(l, r) = l, r : sp.stereoize(fi.lowpass(3, lowf)) : _, _;
high(l, r) = l, r : sp.stereoize(fi.highpass(3, highf)) : _, _;
mid(l, r) = l, r : sp.stereoize(fi.lowpass(3, highf) : fi.highpass(3, lowf)) : _, _;

process = _, _ <: low, mid, high;