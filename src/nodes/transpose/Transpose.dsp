import("stdfaust.lib");

window = vslider( "Window", 64, 1, 4096, 1);
lowpass = vslider( "Fade", 32, 1, 4096, 1);
pitch = vslider( "Pitch", 0, -12, 12, 0.001) : si.smooth(0.999);

process = sp.stereoize(ef.transpose(window, lowpass, pitch));