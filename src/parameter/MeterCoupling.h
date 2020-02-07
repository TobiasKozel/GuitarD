#pragma once
#include "IPlugParameter.h"
namespace guitard {
  struct MeterCoupling {
    sample* value;
    const char* name;
    sample min;
    sample max;
  };
}