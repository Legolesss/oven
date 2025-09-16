#pragma once
struct ITempSensor {
  virtual ~ITempSensor() = default;
  // return NaN on invalid
  virtual double read_celsius() = 0;
};
