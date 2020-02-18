
import("stdfaust.lib");

maxf = 20000;
minf = 20;

highf = vslider("highF", minf, minf, maxf, 1) : si.smooth(0.999);
lowf = vslider("lowF", maxf, minf, maxf, 1) : si.smooth(0.999);
width = vslider("Width", maxf, minf, maxf, 1) : si.smooth(0.999);
ratio = vslider("ratio", 1, 0.01, 10, 0.01) : si.smooth(0.999);
speed = vslider("speed", 2, 0.01, 20, 0.01) : si.smooth(0.999);
depth = vslider("Depth",1, 0, 2, 0.001);
level = vslider("Wah",1, 0, 1, 0.001);
fb = vslider("feedback",0, -1, 1, 0.001);


process = sp.stereoize(pf.phaser2_mono(2, 0, width, lowf, ratio, highf, speed, depth, fb, 0));

