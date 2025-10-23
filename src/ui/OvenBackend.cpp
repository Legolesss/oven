#include "OvenBackend.h"                                  // our header
#include "../core/StateMachine.h"                         // your state machine declaration
#include <QDebug>                                         // optional: qDebug() prints to console

using namespace std::chrono;                              // shorten chrono type names

// Constructor: we receive a pointer to an already-created StateMachine.
// We wire a QTimer to periodically call tick() and set an initial status text.
OvenBackend::OvenBackend(StateMachine* sm, QObject* parent)
  : QObject(parent)                                       // initialize QObject base with optional parent
  , sm_(sm)                                              // stash the state machine pointer for later use
{
  // Defensive check: if no state machine is provided, warn (prevents null deref).
  if (!sm_) {
    qWarning() << "OvenBackend constructed with null StateMachine*.";
  }

  // Configure our periodic timer that will drive sm_->tick(now).
  tick_.setInterval(50);                                  // 50 ms ≈ 20 Hz update rate (adjust as you like)
  tick_.setTimerType(Qt::PreciseTimer);                   // request best-effort timing (no guarantees)
  QObject::connect(&tick_, &QTimer::timeout,              // when the timer fires...
                   this, &OvenBackend::onTick);           // ...call our onTick() method
  tick_.start();                                          // begin ticking as soon as backend is constructed

  // Show something sensible at boot.
  setStatus("Idle");                                      // default text until a real state query is wired
}

// Called by QML when the user taps the "Start" button.
void OvenBackend::start() {
  if (!sm_) return;                                       // safety: nothing to call if sm_ is null
  sm_->command_start();                                   // call YOUR real API (from the code you posted)
  setStatus("Running");                                   // simple reflected status (optional cosmetic)
}

// Called by QML when the user taps the "Stop" button.
void OvenBackend::stop() {
  if (!sm_) return;                                       // safety: nothing to call if sm_ is null
  sm_->command_stop();                                    // call YOUR real API (from the code you posted)
  setStatus("Stopped");                                   // simple reflected status (optional cosmetic)
}

// Internal helper to update the exported 'status' string and notify QML if it changed.
void OvenBackend::setStatus(const QString& s) {
  if (s == status_) return;                               // no-op if identical (avoid spurious updates)
  status_ = s;                                            // store new text
  emit statusChanged();                                   // tell QML bindings to refresh
}

// Periodic tick handler: advances your StateMachine.
// This uses std::chrono::steady_clock::now() because your tick() expects that type.
void OvenBackend::onTick() {
  if (!sm_) return;                                       // safety
  const auto now = steady_clock::now();                   // get a monotonic time point
  sm_->tick(now);                                         // drive YOUR per-loop logic (reads sensors, etc.)
  // OPTIONAL: if you later expose a read-only accessor for the real enum,
  //           you can map it here to a nicer status string and call setStatus(...).
}
