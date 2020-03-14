#pragma once
#include <string>
namespace guitard {
  typedef
#ifdef SAMPLE_TYPE_FLOAT
  float
#else
  double
#endif
  sample;

  typedef std::string String;

  static const int kNoParameter = -1;
  static const int kNoValIdx = -1;
  static const double PI = 3.14159265358979323846;
  String HOME_PATH; // This is a global Variable
}