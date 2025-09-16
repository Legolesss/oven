#pragma once
enum class FanMode { Low, High, Off };
struct IFan {
  virtual ~IFan() = default;
  virtual void set(FanMode m) = 0;
  virtual FanMode get() const = 0;
};
