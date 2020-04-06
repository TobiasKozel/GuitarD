/**
 * Simple little benchmark wich tests a preset at different blocksizes
 * on a 20 second audio snippet (silence, but shouldn't matter)
 */

// #include "./compile_unit/GHeadlessUnit.h" // you can also use the compile_unit version and link against it to speed up things
#include "./GHeadless.h"
#include <fstream>
#include <string>
#include <chrono>
#include <iostream>

int main() {
  const int samplerate = 44100;
  const int samplesLeftTotal = samplerate * 20;
  int sizes[] = { 512, 256, 128, 64, 32, 16, 8, 4, 2, 1 };
  const int channels = 2;
  const int maxBuffer = 512;

  guitard::sample buffers[channels * 2][maxBuffer];
  guitard::sample* in[channels] = { buffers[0], buffers[1] };
  guitard::sample* out[channels] = { buffers[2], buffers[3] };

  guitard::GuitarDHeadless headless;
  headless.setConfig(samplerate, channels, channels);
  std::ifstream file("../../thirdparty/soundwoofer/dummy_backend/presets/Budged EBow.json");
  std::string contents((std::istreambuf_iterator<char>(file)), std::istreambuf_iterator<char>());
  headless.load(contents.c_str());
  std::cout << "\nBlock\tms\n";

  for (auto i : sizes) {
    std::chrono::nanoseconds total;
    auto start = std::chrono::high_resolution_clock::now();
    int samplesLeft = samplesLeftTotal;
    while (samplesLeft > 0) {
      headless.process(in, out, i);
      samplesLeft -= i;
    }
    auto end = std::chrono::high_resolution_clock::now();
    total = end - start;
    auto ms = std::chrono::duration_cast<std::chrono::milliseconds>(total);
    std::cout << i << "\t" << ms.count() << "\n";
  }

  return 0;
}