import("stdfaust.lib");

maxDelay = 0.01; // in ms
delayl = vslider("DelayL", 0.001, 0, maxDelay, 0.0001) * ma.SR : si.smooth(0.999);
delayr = vslider("DelayR", 0.001, 0, maxDelay, 0.0001) * ma.SR : si.smooth(0.999);
feedback = vslider("Feedback", 0, 0, 1, 0.001) * ma.SR : si.smooth(0.999);

process = pf.flanger_stereo(maxDelay * ma.SR, delayl, delayr, 1, feedback, 0);