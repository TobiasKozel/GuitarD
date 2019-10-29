import("stdfaust.lib");

panVal = vslider( "Panning", 0, -1, 1, 0.01);
widthVal = vslider( "Width", 1, 0, 2, 0.01);
postPan = vslider( "Post panning", 1, 0, 1, 1);

pan(l, r) = l * (-panVal + 1, 1 : min), r * (panVal + 1, 1 : min);
midSide(l, r) = (l + r) * 0.5, (l - r) * 0.5;
leftRigh(m, s) = (m + s), (m - s);
width(l, r) = midSide(l, r) : _ * (-widthVal + 2, 1 : min), _ * (widthVal, 1 : min) : leftRigh;
panWidth = pan : width;
widthPan = width : pan;
process = _, _ <: panWidth, widthPan : ba.select2stereo(postPan);