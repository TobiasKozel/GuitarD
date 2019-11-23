import("stdfaust.lib");
drive = vslider( "Drive", 0, 0, 1, 0.01);
offset = vslider( "Offset", 0, 0, 1, 0.01);
postgain = vslider( "Post gain", 0.5, 0, 1, 0.01);

meter = _ <: attach(_,abs : vbargraph("Level",0,1)) :> _;
cubicnl = *(pregain) : meter : +(offset) : clip(-1,1) : cubic
with {
    pregain = pow(10.0,2*drive);
    clip(lo,hi) = min(hi) : max(lo);
    cubic(x) = x - x*x*x/3;
};

dri = cubicnl : ef.cubicnl_nodc(drive, offset) * postgain;
process = dri, dri;