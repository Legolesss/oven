#pragma once
#include <chrono>

enum class State { Idle, Warming, Ready, Curing, Shutdown, Fault };

struct Params {
  double air_target_c      = 200.0;
  double air_hysteresis_c  = 3.0;

  double part_target_c     = 190.0;
  double part_hysteresis_c = 3.0;

  int    dwell_seconds     = 20 * 60;

  // For detecting part insertion: IR must DROP by at least this much
  // (because we go from hot wall → cold part).
  double ir_drop_delta_c   = 40.0;   // e.g. 40°C drop

  // Oven must be hot enough (IR baseline) before we trust a drop as a "part insert".
  double part_min_valid_c  = 120.0;  // wall is hot → drop is meaningful
};
