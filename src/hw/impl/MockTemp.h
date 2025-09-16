#pragma once
#include "../ITempSensor.h"
#include <cmath>
class MockTemp : public ITempSensor {
  double t_ = 25.0;
public:
  void inject(double t){ t_ = t; }
  double read_celsius() override { return t_; }
};
