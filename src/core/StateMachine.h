#pragma once
#include <chrono>
#include <limits>            // std::numeric_limits for quiet_NaN
#include "../hw/IHeater.h"
#include "../hw/IFan.h"
#include "../hw/ITempSensor.h"
#include "Events.h"

class StateMachine {
public:
  // Inject: AIR (thermocouple) and IR (part sensor); plus heater & fan
  StateMachine(Params p, ITempSensor& air_sensor, ITempSensor& part_sensor,
               IHeater& h, IFan& f);

  // Called once per loop with a monotonic clock time
  void tick(std::chrono::steady_clock::time_point now);

  // Operator/GUI commands
  void command_start();        // Idle -> Warming
  void command_stop();         // any  -> Shutdown
  void command_clearFault();   // Fault -> Idle (if safe)

  // Manual State Commands
  void command_enterIdle(); 
  void command_enterWarming();
  void command_enterReady();   
  void command_enterCuring();
  void command_enterShutdown();
  void command_enterFault();   


  // Inputs (wired later; keyboard in laptop tests)
  void setFault(bool f)       { fault_ = f; }
  void setDoorOpen(bool open) { door_open_ = open; }

  // ---------- Status for UI ----------
  State  state() const { return st_; }

  // Raw readings
  double air_c() const { return last_air_c_; }    // AIR (thermocouple)
  double ir_c()  const { return last_part_c_; }   // IR raw (wall before detection; part after)

  // Logical "part temperature" (only meaningful after detection)
  double part_c() const {
    return part_detected_ ? last_part_c_ : std::numeric_limits<double>::quiet_NaN();
  }

  bool   part_detected() const { return part_detected_; }
  int    seconds_left()  const;

private:
  // State transitions and per-state updates
  void enter(State s);   // one-time "on entry"
  void update_idle();
  void update_warming();                                    // AIR control; fan HIGH
  void update_ready(std::chrono::steady_clock::time_point now);   // wait for drop + target
  void update_curing(std::chrono::steady_clock::time_point now);  // PART control; dwell
  void update_shutdown();

  // IR drop detector (tracks baseline while in Ready; latches detection on big drop)
  void update_part_detection();

  // Config & hardware
  Params       P_;
  ITempSensor& air_;
  ITempSensor& part_;
  IHeater&     heater_;
  IFan&        fan_;

  // Runtime state & flags
  State st_{State::Idle};
  bool  fault_{false};
  bool  door_open_{false};

  // Cached readings
  double last_air_c_{ std::numeric_limits<double>::quiet_NaN() };
  double last_part_c_{ std::numeric_limits<double>::quiet_NaN() };

  // Curing dwell bookkeeping
  bool cure_timer_running_{false};
  std::chrono::steady_clock::time_point cure_ends_{};

  // IR drop detection (EWMA baseline)
  bool   part_detected_{false};
  double part_baseline_c_{ std::numeric_limits<double>::quiet_NaN() };
  double part_baseline_alpha_{0.02}; // slow baseline
};
