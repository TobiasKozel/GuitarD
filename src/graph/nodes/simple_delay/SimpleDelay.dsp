import("delays.lib");

maxDelay = 48000;
it = 1024;
dt = vslider("delaytime",1000,0,maxDelay,1);
process = sdelay(maxDelay,it,dt), sdelay(maxDelay,it,dt);