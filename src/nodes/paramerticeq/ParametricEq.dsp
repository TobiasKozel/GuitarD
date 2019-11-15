import("stdfaust.lib");

minb = -50;
maxb = 30;
maxq = 4000;
maxf = 20000;
minf = 20;

highf = vslider("highF", minf, minf, maxf, 1) : si.smooth(0.999);
highq = vslider("highQ", 1, 0.1, 5, 0.1) : si.smooth(0.999);

peak1 = vslider("peak1", 0, minb, maxb, 1) : si.smooth(0.999);
f1 = vslider("f1", 300, minf, maxf, 1) : si.smooth(0.999);
q1 = vslider("q1", 300, 0, maxq, 1) : si.smooth(0.999);

peak2 = vslider("peak2", 0, minb, maxb, 0.1) : si.smooth(0.999);
f2 = vslider("f2", 3000, minf, maxf, 1) : si.smooth(0.999);
q2 = vslider("q2", 500, 0, maxq, 1) : si.smooth(0.999);

lowf = vslider("lowF", maxf, minf, maxf, 1) : si.smooth(0.999);
lowq = vslider("lowQ", 1, 0.1, 5, 0.1) : si.smooth(0.999);

eq = fi.resonhp(highf, highq, 1) : fi.peak_eq(peak1, f1, q1) : fi.peak_eq(peak2, f2, q2) : fi.resonlp(lowf, lowq, 1);

process = sp.stereoize(eq);
