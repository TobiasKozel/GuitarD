import("stdfaust.lib");

l(l, r) = l, l;
r(l, r) = r, r;
m(l, r) = (l + r) * 0.5, (l + r) * 0.5;
s(l, r) = (l - r) * 0.5, (l - r) * 0.5;

process = _, _ <: l, r, m, s;