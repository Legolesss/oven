#pragma once
struct IHeater {
  virtual ~IHeater() = default;
  virtual void set(bool on) = 0;
  virtual bool get() const = 0;
};
