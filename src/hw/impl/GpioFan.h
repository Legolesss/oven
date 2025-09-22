#pragma once
#include <gpiod.h>
#include <stdexcept>
#include "../IFan.h"   // must define set(bool), get() const

class GpioFan final : public IFan {
  gpiod_chip* chip_;
  gpiod_line* line_;
  bool state_{false};
  bool activeHigh_;
public:
  GpioFan(const char* chipName, unsigned lineOffset, bool activeHigh = true)
    : activeHigh_(activeHigh) {
    chip_ = gpiod_chip_open_by_name(chipName);
    if (!chip_) throw std::runtime_error("failed to open chip");
    line_ = gpiod_chip_get_line(chip_, lineOffset);
    if (!line_) throw std::runtime_error("failed to get line");
    if (gpiod_line_request_output(line_, "oven", 0) < 0)
      throw std::runtime_error("failed to request line as output");
  }

  void set(bool on) override {
    state_ = on;
    int phys = activeHigh_ ? (on ? 1 : 0) : (on ? 0 : 1);
    gpiod_line_set_value(line_, phys);
  }

  bool get() const override { return state_; }

  ~GpioFan() {
    if (line_) gpiod_line_release(line_);
    if (chip_) gpiod_chip_close(chip_);
  }
};
