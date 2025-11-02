#pragma once
struct IRelay {
  virtual ~IRelay() = default;
  virtual void set(bool on) = 0;
  virtual bool get() const = 0;
};
