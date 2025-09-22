#pragma once
struct IFan {
  virtual ~IFan() = default;
  virtual void set(bool on) = 0;
  virtual bool get() const = 0;
};
