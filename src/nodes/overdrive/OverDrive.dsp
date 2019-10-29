import("stdfaust.lib");

drive = vslider("Drive", 1, 1, 20, 0.1);
f = drive * -0.5 : ba.db2linear;

overdrive(x) = (x*(abs(x) + drive)/(x*x + (drive-1)*abs(x) + 1)) * f;

process = sp.stereoize(overdrive);
