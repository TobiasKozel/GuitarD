#pragma once
#include "IPlugParameter.h"
struct MeterCoupling {
  iplug::sample* value;
  const char* name;
  iplug::sample min;
  iplug::sample max;
};