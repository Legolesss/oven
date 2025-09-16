#pragma once
#include "../IFan.h"
class MockFan : public IFan {
  FanMode m_ = FanMode::Off;
public:
  void set(FanMode m) override { m_ = m; }
  FanMode get() const override { return m_; }
};
