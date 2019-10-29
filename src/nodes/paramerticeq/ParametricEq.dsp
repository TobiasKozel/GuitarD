import("stdfaust.lib");

minb = -20;
maxb = 20;
maxq = 4000;
maxf = 17000;

highf = ba.lin2LogGain(vslider("highF", 0, 0, 1, 0.01)) * maxf + 20;
highq = vslider("highQ", 1, 0.1, 5, 0.1);

peak1 = vslider("peak1", 0, minb, maxb, 0.1);
f1 = ba.lin2LogGain(vslider("f1", 0.2, 0, 1, 0.01)) * maxf + 20;
q1 = vslider("q1", 300, 0, maxq, 1);

peak2 = vslider("peak2", 0, minb, maxb, 0.1);
f2 = ba.lin2LogGain(vslider("f2", 0.5, 0, 1, 0.01)) * maxf + 20;
q2 = vslider("q2", 500, 0, maxq, 1);

lowf = ba.lin2LogGain(vslider("lowF", 1, 0, 1, 0.01)) * maxf + 20;
lowq = vslider("lowQ", 1, 0.1, 5, 0.1);

eq = fi.resonhp(highf, highq, 1) : fi.peak_eq(peak1, f1, q1) : fi.peak_eq(peak2, f2, q2) : fi.resonlp(lowf, lowq, 1);

process = sp.stereoize(eq);
