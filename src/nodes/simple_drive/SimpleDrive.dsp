import("stdfaust.lib");
drive = vslider( "Drive", 0, 0, 1, 0.01);
offset = vslider( "Offset", 0, 0, 1, 0.01);
postgain = vslider( "Post gain", 0.5, 0, 1, 0.01);
dri = ef.cubicnl(drive, offset) : ef.cubicnl_nodc(drive, offset) * postgain;
process = dri, dri;