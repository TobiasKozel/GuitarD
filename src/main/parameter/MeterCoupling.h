#pragma once
namespace guitard {
  struct MeterCoupling {
    sample* value;
    const char* name;
    sample min;
    sample max;
  };
}