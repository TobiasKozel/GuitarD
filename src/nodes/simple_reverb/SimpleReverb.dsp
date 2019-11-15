import("stdfaust.lib");


pre = vslider( "Predeley", 0, 0, 100, 0.1);
f1 = vslider( "Band1", 1000, 20, 20000, 1);
t60dc = vslider( "Decay Band1", 1, 0.01, 10, 0.01);
f2 = vslider( "Band2", 8000, 20, 20000, 1);
t60m = vslider( "Decay Band2", 2, 0.01, 10, 0.01);

process = _,_ : re.zita_rev1_stereo(pre, f1, f2, t60dc, t60m, ma.SR) : _,_;