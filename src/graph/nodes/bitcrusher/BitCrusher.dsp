import("stdfaust.lib");

bits = 2, vslider( "Bits", 16, 0.1, 16, 0.01) : pow;

sampleFactor = vslider("Downsampling Factor",1, 1, 80, 1) : int;

mix = vslider("Mix",1, 0, 1, 0.01);

counter(x) = int(_)~(_ <: ba.if(_<(x-1) , _+1 , 0));
dsWet(s,c) = _~(ba.if(c == 0 , s , _));
ds(s) = ba.if(sampleFactor > 1 , dsWet(s,counter(sampleFactor)) , s);
depthReduction(x) = rint(x * bits) / bits;
dryWet(m, a, b) = a * m + b * (1 - m);
main(a) = dryWet(mix, (depthReduction(ds(a))), a);

process = sp.stereoize(main);