#include "OvenBackend.h"
#include "ui/ThkaPoller.h"
#include "hw/impl/ThkaRs485Temp.h"
#include <QDebug>
#include <chrono>

using Clock = std::chrono::steady_clock;

OvenBackend::OvenBackend(StateMachine* sm, QObject* parent)
  : QObject(parent), sm_(sm) {
    if (!sm_) qWarning() << "OvenBackend constructed with null StateMachine*.";

    tick_.setInterval(50);
    tick_.setTimerType(Qt::PreciseTimer);
    connect(&tick_, &QTimer::timeout, this, &OvenBackend::onTick);
    tick_.start();

    setStatus("Idle");
}

void OvenBackend::setThka(ThkaRs485Temp* thka) {
    thka_ = thka;
    if (!thka_) return;

    poller_ = new ThkaPoller(thka_);
    poller_->moveToThread(&thkaThread_);

    connect(&thkaThread_, &QThread::finished, poller_, &QObject::deleteLater);
    connect(&thkaThread_, &QThread::started,  poller_, &ThkaPoller::start);
    connect(poller_, &ThkaPoller::polled,     this,    &OvenBackend::onThkaUpdate);

    thkaThread_.start();

}

void OvenBackend::onThkaUpdate(const QVariantList& temps) {
    if (temps == thkaTemps_) return;
    thkaTemps_ = temps;
    emit thkaTempsChanged();
}

void OvenBackend::onTick() {
    if (sm_) sm_->tick(Clock::now());
}

// your enterIdle/enterWarming/... methods and setStatus() stay as they are
void OvenBackend::enterIdle() {
    if (!sm_) return;                                       // safety: nothing to call if sm_ is null
    sm_->command_enterIdle();                                   // call YOUR real API (from the code you posted)
    setStatus("Idle");                                   // simple reflected status (optional cosmetic)
}
void OvenBackend::enterWarming() {
    if (!sm_) return;                                       // safety: nothing to call if sm_ is null
    sm_->command_enterWarming();                                   // call YOUR real API (from the code you posted)
    setStatus("Warming");                                   // simple reflected status (optional cosmetic)
}
void OvenBackend::enterReady() {
    if (!sm_) return;                                       // safety: nothing to call if sm_ is null
    sm_->command_enterReady();                                   // call YOUR real API (from the code you posted)
    setStatus("Ready");                                   // simple reflected status (optional cosmetic)
}
void OvenBackend::enterCuring() {
    if (!sm_) return;                                       // safety: nothing to call if sm_ is null
    sm_->command_enterCuring();                                   // call YOUR real API (from the code you posted)
    setStatus("Curing");                                   // simple reflected status (optional cosmetic)       
}
void OvenBackend::enterShutdown() {
    if (!sm_) return;                                       // safety: nothing to call if sm_ is null
    sm_->command_enterShutdown();                                   // call YOUR real API (from the code you posted)
    setStatus("Shutdown");                                   // simple reflected status (optional cosmetic)
}
void OvenBackend::enterFault() {
    if (!sm_) return;                                       // safety: nothing to call if sm_ is null
    sm_->command_enterFault();                                   // call YOUR real API (from the code you posted)
    setStatus("Fault");                                   // simple reflected status (optional cosmetic)
}


// Internal helper to update the exported 'status' string and notify QML if it changed.
void OvenBackend::setStatus(const QString& s) {
  if (s == status_) return;                               // no-op if identical (avoid spurious updates)
  status_ = s;                                            // store new text
  emit statusChanged();                                   // tell QML bindings to refresh
}


