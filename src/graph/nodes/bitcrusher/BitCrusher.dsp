import("stdfaust.lib");

bits = 2, vslider( "Bits", 8, 0.1, 16, 0.01) : pow;

bitcrusher(x) = rint(x * bits) / bits;

process = bitcrusher, bitcrusher;