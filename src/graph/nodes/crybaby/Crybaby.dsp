import("stdfaust.lib");

wah = vslider( "Wah", 0, 0, 1, 0.01);
process = ve.crybaby(wah);