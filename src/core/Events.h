#pragma once
#include <chrono>

enum class State { Idle, Warming, Ready, Curing, Shutdown, Fault, AutoCureComplete };

enum class OperatingMode { Manual, Auto };

struct Params {
  double air_target_c      = 200.0;
  double air_hysteresis_c  = 3.0;

  double part_target_c     = 190.0;
  double part_hysteresis_c = 3.0;

  int    dwell_seconds     = 5 * 60;

  // For detecting part insertion: IR must DROP by at least this much
  double ir_drop_delta_c   = 40.0;   // e.g. 40°C drop

  // Oven must be hot enough (IR baseline) before we trust a drop as a "part insert".
  double part_min_valid_c  = 100.0;  // wall is hot → drop is meaningful
  
  // Auto mode parameters
  double auto_target_temp_tolerance_c = 10.0;  // ±10°C tolerance for auto mode
  int    auto_cure_duration_seconds   = 5 * 60; // 5 minutes cure time
};