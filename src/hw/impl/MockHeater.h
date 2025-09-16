#pragma once
#include "../IHeater.h"
class MockHeater : public IHeater {
  bool on_ = false;
public:
  void set(bool on) override { on_ = on; }
  bool get() const override { return on_; }
};
