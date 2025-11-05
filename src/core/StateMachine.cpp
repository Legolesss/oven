#include "StateMachine.h"
#include <cmath>

StateMachine::StateMachine(Params p, ITempSensor& air_sensor, ITempSensor& part_sensor,
                           IRelay& f2, IRelay& f, IRelay& greenL, IRelay& redL, IRelay& amberL,
                           IRelay& buzzerL, IRelay& contactor)
  : P_(p), air_(air_sensor), part_(part_sensor), fan2_(f2), fan_(f), 
    greenL_(greenL), redL_(redL), amberL_(amberL), buzzerL_(buzzerL), contactor_(contactor)
{
  enter(State::Idle);
}

void StateMachine::enter(State s){
  st_ = s;
  switch(s){
    case State::Idle:
      cure_timer_running_ = false;
      part_detected_ = false;
      
      greenL_.set(false); 
      redL_.set(false);    
      amberL_.set(false);
      buzzerL_.set(false);
      contactor_.set(false);
      
      // Reset auto mode state when entering idle
      if (mode_ == OperatingMode::Auto) {
        auto_part_at_temp_ = false;
        auto_cure_complete_ = false;
      }
      break;
      
    case State::Warming:
      fan_.set(true);
      fan2_.set(true);
      contactor_.set(true);
      amberL_.set(true);
      greenL_.set(false);
      redL_.set(false);
      buzzerL_.set(false);
      break;

    case State::Ready:
      contactor_.set(true);  
      fan_.set(true);
      fan2_.set(true);
      part_detected_ = false;
      part_baseline_c_ = last_part_c_;
      
      amberL_.set(true);
      greenL_.set(false);
      redL_.set(false);
      buzzerL_.set(false);
      break;

    case State::Curing:
      contactor_.set(true);
      fan2_.set(true);
      fan_.set(true);
      
      amberL_.set(true);
      greenL_.set(true);
      redL_.set(false);
      buzzerL_.set(false);
      break;

    case State::Shutdown:
      contactor_.set(false);
      fan2_.set(false);
      fan_.set(false);
      cure_timer_running_ = false;
      
      amberL_.set(true);
      greenL_.set(true);
      redL_.set(true);
      buzzerL_.set(true);
      break;

    case State::Fault:
      contactor_.set(false);
      fan2_.set(true);
      fan_.set(true);
      cure_timer_running_ = false;
      
      redL_.set(true);
      amberL_.set(false);
      greenL_.set(false);
      buzzerL_.set(true);
      break;
      
    case State::AutoCureComplete:
      // Turn everything off except fans (for cooling)
      contactor_.set(false);
      fan2_.set(true);
      fan_.set(true);
      cure_timer_running_ = false;
      
      // Only green light on - stack light will flash automatically
      greenL_.set(true);
      redL_.set(false);
      amberL_.set(false);
      buzzerL_.set(true);  
      
      // Mark cure as complete for UI
      auto_cure_complete_ = true;
      break;
  }
}

// Manual mode commands
void StateMachine::command_start(){
  if(st_ == State::Idle) enter(State::Warming);
}

void StateMachine::command_stop(){
  enter(State::Shutdown);
}

void StateMachine::command_clearFault(){
  if(st_ == State::Fault && !fault_) enter(State::Idle);
}

void StateMachine::command_enterIdle()     { mode_ = OperatingMode::Manual; enter(State::Idle); }
void StateMachine::command_enterWarming()  { mode_ = OperatingMode::Manual; enter(State::Warming); }
void StateMachine::command_enterReady()    { mode_ = OperatingMode::Manual; enter(State::Ready); }
void StateMachine::command_enterCuring()   { mode_ = OperatingMode::Manual; enter(State::Curing); }
void StateMachine::command_enterShutdown() { mode_ = OperatingMode::Manual; enter(State::Shutdown); }
void StateMachine::command_enterFault()    { mode_ = OperatingMode::Manual; enter(State::Fault); }

// Auto mode commands
void StateMachine::command_startAutoMode(double target_temp) {
  mode_ = OperatingMode::Auto;
  auto_target_temp_ = target_temp;
  auto_part_at_temp_ = false;
  auto_cure_complete_ = false;
  part_detected_ = false;
  
  // Set target for both air and part sensors
  P_.air_target_c = target_temp;
  P_.part_target_c = target_temp;
  
  // Start warming
  enter(State::Warming);
}

void StateMachine::command_cancelAutoMode() {
  mode_ = OperatingMode::Manual;
  auto_part_at_temp_ = false;
  auto_cure_complete_ = false;
  enter(State::Idle);
}

void StateMachine::command_acknowledgeAutoCureComplete() {
  // User clicked OK on the dialog
  if (st_ == State::AutoCureComplete) {
    auto_cure_complete_ = false;
    enter(State::Idle);
  }
}

int StateMachine::seconds_left() const {
  if(!cure_timer_running_) return 0;
  auto now  = std::chrono::steady_clock::now();
  auto left = std::chrono::duration_cast<std::chrono::seconds>(cure_ends_ - now).count();
  return left > 0 ? (int)left : 0;
}

void StateMachine::tick(std::chrono::steady_clock::time_point now){
  last_air_c_  = air_.read_celsius();
  last_part_c_ = part_.read_celsius();

  if(std::isnan(last_air_c_) || std::isnan(last_part_c_) || fault_){
    enter(State::Fault);
    return;
  }

  update_part_detection();

  // Route to auto or manual state handlers
  if (mode_ == OperatingMode::Auto) {
    switch(st_){
      case State::Idle:            update_idle();                      break;
      case State::Warming:         update_auto_warming();              break;
      case State::Ready:           update_auto_ready(now);             break;
      case State::Curing:          update_auto_curing(now);            break;
      case State::Shutdown:        update_shutdown();                  break;
      case State::Fault:           /* wait for clear */                break;
      case State::AutoCureComplete: update_auto_cure_complete(now);    break;
    }
  } else {
    switch(st_){
      case State::Idle:            update_idle();                      break;
      case State::Warming:         update_warming();                   break;
      case State::Ready:           update_ready(now);                  break;
      case State::Curing:          update_curing(now);                 break;
      case State::Shutdown:        update_shutdown();                  break;
      case State::Fault:           /* wait for clear */                break;
      case State::AutoCureComplete: /* shouldn't happen in manual */   break;
    }
  }
}

// Manual mode updates (unchanged)
void StateMachine::update_idle(){
  // Safety: turn on fans if oven is still hot
  if(last_air_c_ > 80.0) {
    fan_.set(true);
    fan2_.set(true);
  } else {
    fan_.set(false);
    fan2_.set(false);
  }
  
  // Heater always off in idle
  contactor_.set(false);
}

void StateMachine::update_warming(){
  if(last_air_c_ < P_.air_target_c - P_.air_hysteresis_c) fan2_.set(true);
  if(last_air_c_ > P_.air_target_c + P_.air_hysteresis_c) fan2_.set(false);
  
  if(last_air_c_ >= P_.air_target_c){
    enter(State::Ready);
  }
}

void StateMachine::update_ready(std::chrono::steady_clock::time_point now){

  if(part_detected_ ){
    enter(State::Curing);
  }
}

void StateMachine::update_curing(std::chrono::steady_clock::time_point now){

  double pc = part_c();
  bool part_hot = !std::isnan(pc) && pc >= P_.part_target_c;

  if(part_hot){
    cure_timer_running_ = true;
    cure_ends_ = now + std::chrono::seconds(P_.dwell_seconds);
  }

  
  if(cure_timer_running_ && now >= cure_ends_){
    enter(State::Idle);
  }
}

void StateMachine::update_shutdown(){
  fan2_.set(false);
  fan_.set(false);
}

// Auto mode updates
void StateMachine::update_auto_warming(){
  // Heat until air reaches target

  // Once target reached, go to Ready to wait for part
  if(last_air_c_ >= P_.air_target_c){
    enter(State::Ready);
  }
}

void StateMachine::update_auto_ready(std::chrono::steady_clock::time_point now){
  // Wait for part detection (40°C drop in IR)
  // update_part_detection() is called in tick()
  
  if(part_detected_){
    // Part inserted, go to curing
    enter(State::Curing);
    cure_timer_running_ = false; // Don't start timer yet
    auto_part_at_temp_ = false;
  }
}

void StateMachine::update_auto_curing(std::chrono::steady_clock::time_point now){
  // Control temperature based on part sensor
  
  // Check if part is within tolerance (±10°C)
  bool part_in_range = std::abs(last_part_c_ - P_.part_target_c) <= P_.auto_target_temp_tolerance_c;
  
  if(part_in_range){
    // Start timer if not already started
    if(!cure_timer_running_){
      cure_timer_running_ = true;
      cure_ends_ = now + std::chrono::seconds(P_.auto_cure_duration_seconds);
      auto_part_at_temp_ = true;
    }
    
    // Check if cure time is complete
    if(now >= cure_ends_){
      cure_timer_running_ = false;
      // Instead of Shutdown, go to AutoCureComplete
      enter(State::AutoCureComplete);
    }
  } else {
    // Part fell out of range, reset timer
    if(cure_timer_running_){
      cure_timer_running_ = false;
      auto_part_at_temp_ = false;
    }
  }
}

void StateMachine::update_auto_cure_complete(std::chrono::steady_clock::time_point now){
  // Keep fans running for cooling
  fan_.set(true);
  fan2_.set(true);
  
  // Green light stays on - stack light handles flashing automatically
  // Stay in this state until user acknowledges (calls command_acknowledgeAutoCureComplete)
}

void StateMachine::update_part_detection(){
  if (st_ != State::Ready || part_detected_) return;

  if (std::isnan(part_baseline_c_)) part_baseline_c_ = last_part_c_;

  part_baseline_c_ = (1.0 - part_baseline_alpha_) * part_baseline_c_
                   +  part_baseline_alpha_       * last_part_c_;

  const bool wall_hot_enough = (part_baseline_c_ >= P_.part_min_valid_c);
  const double drop         = part_baseline_c_ - last_part_c_;
  const bool   big_drop     = (drop >= P_.ir_drop_delta_c);

  if (wall_hot_enough && big_drop) {
    part_detected_ = true;
  }
}