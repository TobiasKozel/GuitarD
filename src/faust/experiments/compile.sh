#!/bin/sh
faust -cn TestDsp -i -double -scn FaustHeadlessDsp -a ../FaustArchitecture.cpp -o test_faust.h ./test.dsp