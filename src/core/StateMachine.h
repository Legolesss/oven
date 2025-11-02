#pragma once
#include <chrono>
#include <limits>
#include "../hw/IHeater.h"
#include "../hw/IFan.h"
#include "../hw/ITempSensor.h"
#include "../hw/IRelay.h"
#include "Events.h"

class StateMachine {
public:
  StateMachine(Params p, ITempSensor& air_sensor, ITempSensor& part_sensor,
               IRelay& f2, IRelay& f, IRelay& greenL, IRelay& redL, IRelay& amberL,
               IRelay& buzzerL, IRelay& contactor);

  void tick(std::chrono::steady_clock::time_point now);

  // Manual mode commands
  void command_start();
  void command_stop();
  void command_clearFault();
  void command_enterIdle();
  void command_enterWarming();
  void command_enterReady();
  void command_enterCuring();
  void command_enterShutdown();
  void command_enterFault();

  // Auto mode commands
  void command_startAutoMode(double target_temp);
  void command_cancelAutoMode();

  // Inputs
  void setFault(bool f)       { fault_ = f; }
  void setDoorOpen(bool open) { door_open_ = open; }

  // Status for UI
  State  state() const { return st_; }
  OperatingMode mode() const { return mode_; }
  
  double air_c() const { return last_air_c_; }
  double ir_c()  const { return last_part_c_; }
  double part_c() const {
    return part_detected_ ? last_part_c_ : std::numeric_limits<double>::quiet_NaN();
  }

  bool   part_detected() const { return part_detected_; }
  int    seconds_left()  const;
  
  // Auto mode status
  bool   is_auto_mode() const { return mode_ == OperatingMode::Auto; }
  double auto_target_temp() const { return auto_target_temp_; }
  bool   auto_part_at_temp() const { return auto_part_at_temp_; }
  bool   auto_cure_complete() const { return auto_cure_complete_; }

private:
  void enter(State s);
  void update_idle();
  void update_warming();
  void update_ready(std::chrono::steady_clock::time_point now);
  void update_curing(std::chrono::steady_clock::time_point now);
  void update_shutdown();
  void update_part_detection();
  
  // Auto mode specific updates
  void update_auto_warming();
  void update_auto_ready(std::chrono::steady_clock::time_point now);
  void update_auto_curing(std::chrono::steady_clock::time_point now);

  Params       P_;
  ITempSensor& air_;
  ITempSensor& part_;
  IRelay&      fan2_;
  IRelay&      fan_;
  IRelay&      greenL_;
  IRelay&      redL_;
  IRelay&      amberL_;
  IRelay&      buzzerL_;
  IRelay&      contactor_;

  State st_{State::Idle};
  OperatingMode mode_{OperatingMode::Manual};
  bool  fault_{false};
  bool  door_open_{false};

  double last_air_c_{ std::numeric_limits<double>::quiet_NaN() };
  double last_part_c_{ std::numeric_limits<double>::quiet_NaN() };

  bool cure_timer_running_{false};
  std::chrono::steady_clock::time_point cure_ends_{};

  bool   part_detected_{false};
  double part_baseline_c_{ std::numeric_limits<double>::quiet_NaN() };
  double part_baseline_alpha_{0.02};
  
  // Auto mode state
  double auto_target_temp_{200.0};
  bool   auto_part_at_temp_{false};
  bool   auto_cure_complete_{false};
  std::chrono::steady_clock::time_point auto_cure_start_;
};