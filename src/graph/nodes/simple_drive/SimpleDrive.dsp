import("stdfaust.lib");
drive = vslider( "Drive", 0, 0, 1, 0.01);
offset = vslider( "Offset", 0, 0, 20, 0.01);
process = ef.cubicnl(drive, offset) : ef.cubicnl_nodc(drive, offset);