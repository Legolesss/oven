#include "OvenBackend.h"
#include "ui/ThkaPoller.h"
#include "hw/impl/ThkaRs485Temp.h"
#include <QDebug>
#include <QtMath>
#include <chrono>

using Clock = std::chrono::steady_clock;

OvenBackend::OvenBackend(StateMachine* sm, QObject* parent)
  : QObject(parent), sm_(sm) {
    if (!sm_) qWarning() << "OvenBackend constructed with null StateMachine*.";

    emit manualSetpointStatusChanged();

    tick_.setInterval(50);
    tick_.setTimerType(Qt::PreciseTimer);
    connect(&tick_, &QTimer::timeout, this, [this]() {
        onTick();
        updateAutoModeStatus();
    });
    tick_.start();

    setStatus("Idle");
}

void OvenBackend::setThka(ThkaRs485Temp* thka) {
    thka_ = thka;
    if (!thka_) return;

    setManualSetpointStatus("Connected to THKA controller – ready to send setpoints");

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

// ============ MANUAL MODE COMMANDS ============

void OvenBackend::sendManualSetpoint(double value) {
    setManualSetpoint(value);

    if (!thka_) {
        setManualSetpointStatus("❌ Cannot send - THKA not connected");
        return;
    }

    qDebug() << "Sending setpoint" << value << "°C to CH" << manualSetpointChannel_;
    
    bool success = thka_->write_setpoint_celsius(manualSetpointChannel_, value);
    
    if (success) {
        setManualSetpointStatus(QString("✓ Sent %1°C to CH%2")
                                    .arg(value, 0, 'f', 1)
                                    .arg(manualSetpointChannel_));
    } else {
        setManualSetpointStatus(QString("❌ Write failed to CH%1")
                                    .arg(manualSetpointChannel_));
    }
}

void OvenBackend::enterIdle() {
    if (!sm_) return;
    sm_->command_enterIdle();
    setStatus("Idle");
}

void OvenBackend::enterWarming() {
    if (!sm_) return;
    sm_->command_enterWarming();
    setStatus("Warming");
}

void OvenBackend::enterReady() {
    if (!sm_) return;
    sm_->command_enterReady();
    setStatus("Ready");
}

void OvenBackend::enterCuring() {
    if (!sm_) return;
    sm_->command_enterCuring();
    setStatus("Curing");
}

void OvenBackend::enterShutdown() {
    if (!sm_) return;
    sm_->command_enterShutdown();
    setStatus("Shutdown");
}

void OvenBackend::enterFault() {
    if (!sm_) return;
    sm_->command_enterFault();
    setStatus("Fault");
}

// ============ AUTO MODE COMMANDS ============

void OvenBackend::startAutoMode(double targetTemp) {
    if (!sm_) {
        qWarning() << "Cannot start auto mode - StateMachine null";
        return;
    }
    
    if (!thka_) {
        qWarning() << "Cannot start auto mode - THKA not connected";
        return;
    }
    
    qDebug() << "Starting AUTO MODE with target:" << targetTemp << "°C";
    
    // Write target temperature to all THKA channels
    for (int ch = 1; ch <= 6; ++ch) {
        thka_->write_setpoint_celsius(ch, targetTemp);
    }
    
    // Let StateMachine handle the control logic
    sm_->command_startAutoMode(targetTemp);
    
    // Update local state
    autoTargetTemp_ = targetTemp;
    autoModeActive_ = true;
    autoCureComplete_ = false;
    
    emit autoModeActiveChanged();
    emit autoTargetTempChanged();
    emit autoCureCompleteChanged();
    
    setAutoStatus("Starting...");
    setStatus("Auto: Starting");
}

void OvenBackend::cancelAutoMode() {
    if (!sm_) return;
    
    qDebug() << "Cancelling AUTO MODE";
    
    sm_->command_cancelAutoMode();
    
    autoModeActive_ = false;
    autoCureComplete_ = false;
    
    emit autoModeActiveChanged();
    emit autoCureCompleteChanged();
    
    setAutoStatus("Cancelled");
    setStatus("Idle");
    setAutoCureTimeLeft(0);
}

void OvenBackend::acknowledgeAutoCureComplete() {
    qDebug() << "User acknowledged cure complete";
    
    // Tell StateMachine to exit AutoCureComplete state
    if (sm_) {
        sm_->command_acknowledgeAutoCureComplete();
    }
    
    // Reset UI state
    autoCureComplete_ = false;
    autoModeActive_ = false;
    
    emit autoCureCompleteChanged();
    emit autoModeActiveChanged();
}

void OvenBackend::updateAutoModeStatus() {
    if (!sm_) return;
    
    // Check if StateMachine is in auto mode
    bool smInAutoMode = sm_->is_auto_mode();
    
    if (autoModeActive_ != smInAutoMode) {
        autoModeActive_ = smInAutoMode;
        emit autoModeActiveChanged();
    }
    
    if (!smInAutoMode) return;
    
    // Get current state and temperatures
    State state = sm_->state();
    double airTemp = thkaTemps_.isEmpty() ? 0.0 : thkaTemps_[0].toDouble();
    double irTemp = (thkaTemps_.size() > 5) ? thkaTemps_[5].toDouble() : 0.0;
    
    // Update status based on state
    switch(state) {
        case State::Warming:
            setAutoStatus(QString("Warming: %1°C / %2°C")
                .arg(airTemp, 0, 'f', 1)
                .arg(sm_->auto_target_temp(), 0, 'f', 1));
            setStatus("Auto: Warming");
            break;
            
        case State::Ready:
            setAutoStatus(QString("Ready - waiting for part (IR: %1°C)")
                .arg(irTemp, 0, 'f', 1));
            setStatus("Auto: Ready");
            break;
            
        case State::Curing:
            if (sm_->auto_part_at_temp()) {
                int timeLeft = sm_->seconds_left();
                setAutoCureTimeLeft(timeLeft);
                
                int mins = timeLeft / 60;
                int secs = timeLeft % 60;
                
                setAutoStatus(QString("Curing: %1:%2 remaining (Part: %3°C)")
                    .arg(mins)
                    .arg(secs, 2, 10, QChar('0'))
                    .arg(irTemp, 0, 'f', 1));
                setStatus("Auto: Curing");
            } else {
                setAutoStatus(QString("Heating part: %1°C / %2°C (±10°C)")
                    .arg(irTemp, 0, 'f', 1)
                    .arg(sm_->auto_target_temp(), 0, 'f', 1));
                setStatus("Auto: Heating");
                setAutoCureTimeLeft(0);
            }
            break;
            
        case State::AutoCureComplete:
            setAutoStatus("✓ Cure Complete! Click OK to continue");
            setStatus("Auto: Complete");
            
            // Set flag for UI popup
            if (!autoCureComplete_) {
                autoCureComplete_ = true;
                emit autoCureCompleteChanged();
            }
            break;
            
        case State::Shutdown:
            // Shouldn't happen in auto mode anymore
            break;
            
        default:
            break;
    }
}

// ============ SETTERS ============

void OvenBackend::setStatus(const QString& s) {
    if (s == status_) return;
    status_ = s;
    emit statusChanged();
}

void OvenBackend::setManualSetpoint(double value) {
    if (qFuzzyCompare(manualSetpoint_, value)) return;
    manualSetpoint_ = value;
    emit manualSetpointChanged();
}

void OvenBackend::setManualSetpointStatus(const QString& status) {
    if (manualSetpointStatus_ == status) return;
    manualSetpointStatus_ = status;
    emit manualSetpointStatusChanged();
}

void OvenBackend::setAutoStatus(const QString& status) {
    if (autoStatus_ == status) return;
    autoStatus_ = status;
    emit autoStatusChanged();
}

void OvenBackend::setAutoCureTimeLeft(int seconds) {
    if (autoCureTimeLeft_ == seconds) return;
    autoCureTimeLeft_ = seconds;
    emit autoCureTimeLeftChanged();
}

void OvenBackend::setAutoCureComplete(bool complete) {
    if (autoCureComplete_ == complete) return;
    autoCureComplete_ = complete;
    emit autoCureCompleteChanged();
}