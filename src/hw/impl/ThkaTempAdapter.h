#pragma once
#include "../ITempSensor.h"
#include <cmath>
#include <atomic>

/**
 * NON-BLOCKING adapter for THKA channels
 * 
 * Returns the last cached value from ThkaPoller.
 * This prevents blocking the GUI thread on every StateMachine tick.
 * 
 * The actual THKA reads happen in the background ThkaPoller thread,
 * and this adapter just returns the latest cached value.
 */
class ThkaTempAdapter : public ITempSensor {
public:
  ThkaTempAdapter(int channel)
    : channel_(channel), cached_temp_(std::nan("")) {}

  // Called by StateMachine in GUI thread - returns cached value instantly
  double read_celsius() override {
    return cached_temp_.load();
  }
  
  // Called by ThkaPoller in worker thread - updates cache
  void update_cache(double temp) {
    cached_temp_.store(temp);
  }
  
  int channel() const { return channel_; }

private:
  int channel_;
  std::atomic<double> cached_temp_;  // Thread-safe cache
};