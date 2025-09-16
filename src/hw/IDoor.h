#pragma once
struct IDiscreteIn {
  virtual ~IDiscreteIn() = default;
  virtual bool active() = 0; // true when tripped/closed etc.
};
