# Headless
The headless.h wraps up all the classes needed to make the DSP run on it's own and allow it to be controlled. All paths in this project are relative so just including it should be enough to make it work in another project.

`benchmark.cpp` and `device.cpp` show how it can be used.

## Compilation
### MSVC
`cl.exe .\benchmark.cpp ws2_32.lib /O2`

### g++
Windows: `g++.exe -O2 --fast-math -ftree-vectorize .\benchmark.cpp -lws2_32`

Linux: `g++ -O2 --fast-math -ftree-vectorize ./benchmark.cpp -pthread`

Compiling device.cpp will also need `-ldl`

Raspberry Pi 2b: `g++ -O3 -ffast-math -ftree-vectorize -mcpu=cortex-a7 -mfpu=neon-vfpv4 -mtune=cortex-a7 ./benchmark.cpp -pthread`

The RPi2b is about 10 times slower than my i7-4790k. For smaller blocksizes it gets much worse though.

### clang++
Linux: `clang++ -Ofast ./benchmark.cpp -pthread`

Mac OS X: `clang++ -std=c++14 -Ofast ./benchmark.cpp -pthread`
