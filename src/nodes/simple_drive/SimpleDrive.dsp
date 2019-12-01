import("stdfaust.lib");
drive = vslider( "Drive", 0, 0, 1, 0.01);
offset = vslider( "Offset", 0, 0, 1, 0.01);
postgain = vslider( "Post gain", 0, -80, 30, 0.1);
highf = vslider( "Bass", 0, -10, 10, 0.1) : si.smooth(0.999);

meter = _ <: attach(_,abs : vbargraph("Level",0,1)) :> _;
cubicnl = *(pregain) : meter : +(offset) : clip(-1,1) : cubic
with {
    pregain = pow(10.0,2*drive);
    clip(lo,hi) = min(hi) : max(lo);
    cubic(x) = x - x*x*x/3;
};
cutoff(f) = (3 * (f - 10)) * (3 * (f - 10));
dri = fi.resonhp(cutoff(highf), 0.5, 1) : cubicnl : ef.cubicnl_nodc(drive, offset) * ba.db2linear(postgain);
process = dri, dri;