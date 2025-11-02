#include "StateMachine.h"
#include <cmath>  // std::isnan

// Constructor: stash refs + defaults, then enter Idle to set safe outputs
StateMachine::StateMachine(Params p, ITempSensor& air_sensor, ITempSensor& part_sensor,
                           IRelay& f2, IRelay& f, IRelay& greenL, IRelay& redL, IRelay& amberL,
                           IRelay& buzzerL, IRelay& contactor)
  : P_(p), air_(air_sensor), part_(part_sensor), fan2_(f2), fan_(f), 
    greenL_(greenL), redL_(redL), amberL_(amberL), buzzerL_(buzzerL), contactor_(contactor)
{
  enter(State::Idle);
}

// One-time "on entry" actions for each state
void StateMachine::enter(State s){
  st_ = s;
  switch(s){
    case State::Idle:
      fan2_.set(true);          // spec: no heat in Idle
      fan_.set(true);      // spec: no fan in Idle
      cure_timer_running_ = false; // no timing
      part_detected_ = false;      // reset detection
      
      greenL_.set(false); 
      redL_.set(false);    
      amberL_.set(false);
      buzzerL_.set(false);
      
      contactor_.set(false);
      break;

    case State::Warming:
      fan_.set(true);     // spec: Warming uses max fan
      contactor_.set(true);

      
      amberL_.set(true);  // indicate Warming
      greenL_.set(false); // indicate Curing
      redL_.set(false);    // indicate Fault
      buzzerL_.set(false);
      // heater toggled in update_warming()
      break;

    case State::Ready:
      contactor_.set(true);  
      fan_.set(true);      // spec: Ready low fan to reduce loss
      part_detected_ = false;      // look for a new spike
      part_baseline_c_ = last_part_c_; // seed baseline if we have a value
      
      amberL_.set(true);  // isame as warming
      greenL_.set(false); // 
      redL_.set(false);    // 
      buzzerL_.set(false);
      
      break;

    case State::Curing:
      contactor_.set(true);
      fan2_.set(true);          // safe-off
      fan_.set(true);      // spec: Curing low fan
      
      amberL_.set(true);  // indicate Warming
      greenL_.set(true); // indicate Curing
      redL_.set(false);    // indicate Fault
      buzzerL_.set(false);
      // heater toggled in update_curing()
      break;

    case State::Shutdown:
      contactor_.set(false);
      amberL_.set(true);  // indicate Warming
      greenL_.set(true); // indicate Curing
      redL_.set(true);    // indicate Fault
      buzzerL_.set(true);

      fan2_.set(false);          // safe-off
      fan_.set(false);     // cool down aggressively
      cure_timer_running_ = false; // stop any dwell
      break;

    case State::Fault:
      contactor_.set(false);
  
      fan2_.set(true);          // kill heat
      fan_.set(true);     // evacuate heat
      cure_timer_running_ = false; // stop timers
      redL_.set(true);    // indicate Fault
      amberL_.set(false);  // indicate Warming
      greenL_.set(false); // indicate Curing
      buzzerL_.set(true);

      break;
  }
}

// Commands
void StateMachine::command_start(){
  if(st_ == State::Idle) enter(State::Warming);
}
void StateMachine::command_stop(){
  enter(State::Shutdown);
}
void StateMachine::command_clearFault(){
  if(st_ == State::Fault && !fault_) enter(State::Idle);
}

  void StateMachine::command_enterIdle()     { enter(State::Idle); }
  void StateMachine::command_enterWarming()  { enter(State::Warming); }
  void StateMachine::command_enterReady()    { enter(State::Ready); }
  void StateMachine::command_enterCuring()   { enter(State::Curing); }
  void StateMachine::command_enterShutdown() { enter(State::Shutdown); }
  void StateMachine::command_enterFault()    { enter(State::Fault); }

// Remaining dwell seconds for UI
int StateMachine::seconds_left() const {
  if(!cure_timer_running_) return 0;
  auto now  = std::chrono::steady_clock::now();
  auto left = std::chrono::duration_cast<std::chrono::seconds>(cure_ends_ - now).count();
  return left > 0 ? (int)left : 0;
}

// Main dispatcher: read sensors, handle faults, per-state update
void StateMachine::tick(std::chrono::steady_clock::time_point now){
  // Read current sensor values and cache
  last_air_c_  = air_.read_celsius();
  last_part_c_ = part_.read_celsius();

  // Safety: invalid sensor or asserted fault forces Fault state
  if(std::isnan(last_air_c_) || std::isnan(last_part_c_) || fault_){
    enter(State::Fault);
    return;
  }

  // Keep IR spike detection updated every loop
  update_part_detection();

  // Per-state logic
  switch(st_){
    case State::Idle:     update_idle();             break;
    case State::Warming:  update_warming();          break;
    case State::Ready:    update_ready(now);         break;
    case State::Curing:   update_curing(now);        break;
    case State::Shutdown: update_shutdown();         break;
    case State::Fault:    /* wait for clear */       break;
  }
}

// IDLE: everything off (explicit)
void StateMachine::update_idle(){
  
}

// WARMING: control on AIR; fan HIGH; go Ready when AIR ≥ target
void StateMachine::update_warming(){
  // Bang-bang with hysteresis on AIR temperature
  if(last_air_c_ < P_.air_target_c - P_.air_hysteresis_c) fan2_.set(true);
  if(last_air_c_ > P_.air_target_c + P_.air_hysteresis_c) fan2_.set(false);
  if(door_open_) fan_.set(false);
  if(!door_open_) fan_.set(true);
  // Transition when oven air is hot enough
  if(last_air_c_ >= P_.air_target_c){
    enter(State::Ready);
  }
}

// READY: hover by AIR; fan LOW; wait for (door open && IR spike && PART ≥ target)
void StateMachine::update_ready(std::chrono::steady_clock::time_point now){


  // Update/drop detection is called in tick(); here we just check conditions:
  // door open, detection latched, and PART (i.e., IR after detection) >= target.
  double pc = part_c(); // will be NaN until detected
  bool part_hot = !std::isnan(pc) && pc >= P_.part_target_c;

  if(!door_open_ && part_detected_ && part_hot){
    enter(State::Curing);
    cure_timer_running_ = true;
    cure_ends_ = now + std::chrono::seconds(P_.dwell_seconds);
  }
}


// CURING: control on PART; fan LOW; finish when dwell timer expires
void StateMachine::update_curing(std::chrono::steady_clock::time_point now){
  // Maintain PART around target with hysteresis
  if(last_part_c_ < P_.part_target_c - P_.part_hysteresis_c) fan2_.set(true);
  if(last_part_c_ > P_.part_target_c + P_.part_hysteresis_c) fan2_.set(false);
  if(door_open_) fan_.set(false);
  if(!door_open_) fan_.set(true);
  // Done when timer elapses
  if(cure_timer_running_ && now >= cure_ends_){
    enter(State::Idle);
  }
}

// SHUTDOWN: safe-off; (optionally auto-return to Idle on cooldown in future)
void StateMachine::update_shutdown(){
  fan2_.set(false);
  fan_.set(false);
  // Optional later: if last_air_c_ < safe threshold → enter(State::Idle);
}

// IR spike detection using a slow EWMA baseline
void StateMachine::update_part_detection(){
  // Only track and detect while in READY, and only until we latch
  if (st_ != State::Ready || part_detected_) return;

  // Initialize baseline (expected "hot wall" value) if needed
  if (std::isnan(part_baseline_c_)) part_baseline_c_ = last_part_c_;

  // EWMA to follow slow changes in the wall temp
  part_baseline_c_ = (1.0 - part_baseline_alpha_) * part_baseline_c_
                   +  part_baseline_alpha_       * last_part_c_;

  // A valid "drop" requires: wall baseline already hot, and a large negative jump
  const bool wall_hot_enough = (part_baseline_c_ >= P_.part_min_valid_c);
  const double drop         = part_baseline_c_ - last_part_c_; // positive if we dropped
  const bool   big_drop     = (drop >= P_.ir_drop_delta_c);

  if (wall_hot_enough && big_drop) { // MAYBE REMOVE DOOR OPEN CONDITION
    // From now on, IR represents PART temperature
    part_detected_ = true;
  }
}

