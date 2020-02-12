#pragma once
#include <string>
namespace guitard {
  typedef
#ifdef GUITARD_HEADLESS
#ifdef GUITARD_SAMPLE_TYPE
    GUITARD_SAMPLE_TYPE
#else
  double
#endif
#else
  iplug::sample
#endif
  sample;

  static const int kNoParameter = -1;
  static const int kNoValIdx = -1;
  static const double PI = 3.14159265358979323846;
  std::string HOME_PATH; // This is a global Variable
}