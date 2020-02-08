#pragma once
#include "./string.h"

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
  String HOME_PATH; // This is a global Variable
}