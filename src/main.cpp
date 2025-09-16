#include <chrono>    // time points & durations
#include <iostream>  // console I/O
#include <thread>    // sleep_for
#include <unistd.h>  // read(), STDIN_FILENO
#include <termios.h> // tcgetattr/tcsetattr for raw input
#include <fcntl.h>   // fcntl for O_NONBLOCK
#include <cmath> // for std::isnan in the status line

#include "core/StateMachine.h"
#include "core/Events.h"

#include "hw/impl/MockHeater.h"
#include "hw/impl/MockFan.h"
#include "hw/impl/MockTemp.h"

// Small RAII helper to put the terminal into raw, noncanonical, no-echo mode
// so we can read single keypresses without needing Enter.
// It also restores the original settings when it goes out of scope.
class StdinRaw {
  termios old_{};
  bool ok_{false};
public:
  StdinRaw() {
    if (tcgetattr(STDIN_FILENO, &old_) == 0) {
      termios raw = old_;
      raw.c_lflag &= ~(ICANON | ECHO);  // noncanonical (no line buffering), no echo
      raw.c_cc[VMIN]  = 0;              // read returns immediately
      raw.c_cc[VTIME] = 0;              // no read timeout (we'll sleep in loop)
      if (tcsetattr(STDIN_FILENO, TCSANOW, &raw) == 0) {
        // also make the file descriptor nonblocking
        int flags = fcntl(STDIN_FILENO, F_GETFL, 0);
        fcntl(STDIN_FILENO, F_SETFL, flags | O_NONBLOCK);
        ok_ = true;
      }
    }
  }
  ~StdinRaw() {
    if (ok_) tcsetattr(STDIN_FILENO, TCSANOW, &old_); // restore terminal
  }
  // Try to read one char; returns true if a char was read
  bool try_get_char(char& c) {
    ssize_t n = ::read(STDIN_FILENO, &c, 1);
    return (n == 1);
  }
};

// Pretty-print helper for the State enum
static const char* to_str(State s){
  switch(s){
    case State::Idle:    return "Idle";
    case State::Warming: return "Warming";
    case State::Ready:   return "Ready";
    case State::Curing:  return "Curing";
    case State::Shutdown:return "Shutdown";
    case State::Fault:   return "Fault";
  }
  return "?";
}

int main(){
  // --- Enable raw keyboard input so single keypresses work without Enter ---
  StdinRaw kb; // RAII object; restores terminal at program exit

  // --- Wire up mocks (swap for Pi drivers later) ---
  MockHeater heater;
  MockFan    fan;
  MockTemp   air;
  MockTemp   part;
  air.inject(25.0);
  part.inject(25.0);

  // Tunables
  Params P;
  P.air_target_c       = 200.0;
  P.part_target_c      = 190.0;
  P.dwell_seconds      = 20;// *60 to turn into minutes

  StateMachine sm(P, air, part, heater, fan);

  // Print help once (not every loop)
    std::cout <<
    "Keys:\n"
    "  s = start (Idle -> Warming)\n"
    "  x = stop  (any  -> Shutdown)\n"
    "  c = clear fault (Fault -> Idle if safe)\n"
    "  f = toggle fault flag\n"
    "  o = toggle door open/closed\n"
    "  A/Z = AIR +5 / -5 C\n"
    "  P/L = IR  +5 / -5 C (raw IR sensor)\n"
    "  B   = IR  -45 C burst (simulate cold part insertion)\n"
    "  q = quit\n\n";

  bool running     = true;
  bool injectFault = false;
  bool doorOpen    = false;

  auto lastPrint = std::chrono::steady_clock::now();

  while(running){
    // --- Nonblocking single-key read (works without pressing Enter) ---
// --- keys (manual driving only) ---
// helper: if NaN, use a sane baseline (25 C)
auto safe = [](double v){ return std::isnan(v) ? 25.0 : v; };

char c;
while (kb.try_get_char(c)) {
  switch(c){
    case 'q': running = false; break;
    case 's': sm.command_start(); break;
    case 'x': sm.command_stop();  break;
    case 'c': sm.command_clearFault(); break;

    case 'f': injectFault = !injectFault; sm.setFault(injectFault); break;
    case 'o': doorOpen = !doorOpen; sm.setDoorOpen(doorOpen); break;

    // AIR manual control (upper + lower)
    case 'A': case 'a': air.inject( safe(sm.air_c()) + 5.0 ); break;
    case 'Z': case 'z': air.inject( safe(sm.air_c()) - 5.0 ); break;

    // IR manual control (raw sensor) (upper + lower)
    case 'P': case 'p': part.inject( safe(sm.ir_c()) + 5.0 ); break;
    case 'L': case 'l': part.inject( safe(sm.ir_c()) - 5.0 ); break;

    // Cold-part insertion: big **negative** step on IR (upper + lower)
    case 'B': case 'b': part.inject( safe(sm.ir_c()) - 45.0 ); break;

    default: break;
  }
}


// --- Tick state machine ---
sm.tick(std::chrono::steady_clock::now());

// *** IMPORTANT: no auto-physics now ***
// Do NOT modify air/part unless user presses keys.

    // --- Advance state machine ---
    sm.tick(std::chrono::steady_clock::now());

    // // --- Simple laptop physics model (unchanged) ---
    // {
    //   const bool on = heater.get();

    //   double air_next  = sm.air_c()  + (on ? +0.8 : -0.2);
    //   if(air_next < 0) air_next = 0;
    //   air.inject(air_next);

    //   double part_next = sm.part_c() + ((air_next - sm.part_c()) * (on ? 0.15 : 0.08));
    //   if(part_next < 0) part_next = 0;
    //   part.inject(part_next);
    // }

    // --- Status line at ~2 Hz (overwrite same line) ---
    // ~2 Hz status line
    auto now = std::chrono::steady_clock::now();
    if(now - lastPrint > std::chrono::milliseconds(500)){
      lastPrint = now;

      const char* fanTxt =
        (fan.get()==FanMode::High ? "HIGH" :
        fan.get()==FanMode::Low  ? "LOW " : "OFF ");

      std::cout << "\x1b[2K\r"
        << "State=" << to_str(sm.state())
        << "  AIR="  << sm.air_c()  << "C"
        << "  IR="   << sm.ir_c()   << "C"
        << "  PART=";

      if(std::isnan(sm.part_c())) std::cout << "N/A";
      else                        std::cout << sm.part_c() << "C";

      std::cout
        << "  Heater=" << (heater.get() ? "ON " : "OFF")
        << "  Fan=" << fanTxt
        << "  Door=" << (doorOpen ? "OPEN " : "CLOSED")
        << "  Detected=" << (sm.part_detected() ? "YES" : "no ")
        << "  DwellLeft=" << sm.seconds_left() << "s"
        << "  Fault=" << (injectFault ? "SET " : "clear")
        << std::flush;
    }

    // ~10 Hz loop
    std::this_thread::sleep_for(std::chrono::milliseconds(100));
  }

  std::cout << "\nBye.\n";
  return 0;
}
