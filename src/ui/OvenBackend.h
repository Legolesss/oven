#pragma once                                              // prevent multiple inclusion of this header

#include <QObject>                                        // base class for Qt objects (signals/slots)
#include <QString>                                        // Qt string class for properties
#include <QTimer>                                         // Qt timer to call tick() periodically
#include <chrono>                                         // std::chrono for time points passed to tick()

// Forward-declare your StateMachine (we include the real header in the .cpp).
class StateMachine;

/**
 * OvenBackend
 * ------------
 * Thin Qt-facing wrapper around your existing StateMachine.
 * - Exposes QML-callable methods: start() / stop().
 * - Exposes a read-only 'status' property so QML can display simple text.
 * - Owns a QTimer that periodically calls sm_.tick(now) at ~20 Hz.
 *   (Later, we can move tick() to a worker thread if your IO blocks.)
 */
class OvenBackend : public QObject {
  Q_OBJECT                                              // enables Qt metaobject (signals/slots/properties)

  // Expose a 'status' text to QML (e.g., "Idle", "Running", "Stopped").
  // QML reads this via 'oven.status'. We notify updates via statusChanged.
  Q_PROPERTY(QString status READ status NOTIFY statusChanged)

public:
  // Construct with a reference to your already-constructed StateMachine.
  // We DO NOT take ownership; we just store the pointer and drive it.
  explicit OvenBackend(StateMachine* sm, QObject* parent = nullptr);

  // Defaulted destructor; the StateMachine belongs to whoever created it (main.cpp).
  ~OvenBackend() override = default;

  // QML can call these functions directly: oven.start(), oven.stop()
  Q_INVOKABLE void start();                              // triggers your StateMachine::command_start()
  Q_INVOKABLE void stop();                               // triggers your StateMachine::command_stop()

  // Getter for the 'status' Q_PROPERTY above.
  QString status() const { return status_; }             // returns current status string

signals:
  // Emitted whenever we change 'status_' so QML can update on screen.
  void statusChanged();

private:
  // Helper to change the internal status string and emit statusChanged() when it actually changed.
  void setStatus(const QString& s);

  // Slot-like function wired to a QTimer; calls sm_.tick(now) on a schedule.
  void onTick();                                         // drives the state machine loop

private:
  StateMachine* sm_ = nullptr;                           // pointer to your core logic (not owned)
  QTimer tick_;                                          // timer that fires e.g., every 50 ms
  QString status_ = "Idle";                              // simple status text exported to QML
};
