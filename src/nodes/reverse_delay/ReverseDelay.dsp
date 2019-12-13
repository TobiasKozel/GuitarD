// Stolen from this gist
// https://gist.github.com/tomoyanonymous/d527fca58e929de6a021565505589406
// 
// Tomoya Matsuura  (github) tomoyanonymous



import("stdfaust.lib");
MAX_DELAY = 48000;
phasor_phase(dtime,phase)= ((os.lf_rawsaw(dtime)+phase)% dtime) :int; //phase shift in sample
indexphasor(dtime,phase) = phasor_phase(dtime*2,phase)<: <=(dtime),(*(-1)+dtime*2),_ : select2; //folded triangle
delay_module(dtime,phase) = rwtable(MAX_DELAY,0.0,indexphasor(dtime,phase):int,_,indexphasor(dtime,phase+1):int):window with{
  window =  *( sin(0.5*ma.PI* phasor_phase(dtime,phase)/dtime));
};//init have to be 0.0 floating point
reversedelay_mono(dtime) = _<:delay_module(dtime,0),delay_module(dtime,dtime/2):>_;
reversedelay_pingpong(dtime,spread,fb) =  (si.bus(2),pingpong_premix :>reversedelay_mono(dtime),reversedelay_mono(dtime)) ~ distribute with{
  distribute = _,_<:*(1-spread),*(spread),*(spread),*(1-spread):+,+:fbgain;
  pingpong_premix = _,_<:_,*(spread),*(0),*(1-spread):>+,+;
  fbgain = *(fb) ,*(fb*si.interpolate(spread,1,0.5));
};
reversedelay_pingpong_mix(dtime,spread,fb,mix) = _,_<:_,_,reversedelay_pingpong(dtime,spread,fb):ro.cross2 : si.interpolate(mix),si.interpolate(mix);
process =reversedelay_pingpong_mix(hslider("delaytime",20000,0,MAX_DELAY-1,1),hslider("spread",0,0,1,0.001),hslider("fb",0,0,0.9999,0.001),hslider("mix",0,0,1,0.001));
//ping pong effect is little weird, maybe feedback occur when spread is close to 1