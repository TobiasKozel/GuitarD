import("stdfaust.lib");

drive = vslider("Drive", 1, 1, 10, 0.1) * 10;
f = drive * -0.2 : ba.db2linear;

overdrive(x) = (x*(abs(x) + drive)/(x*x + (drive-1)*abs(x) + 1)) * f;

process = sp.stereoize(overdrive);
