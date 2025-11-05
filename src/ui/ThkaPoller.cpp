#include "ThkaPoller.h"
#include "hw/impl/ThkaRs485Temp.h"
#include <QDebug>
#include <exception>

ThkaPoller::ThkaPoller(ThkaRs485Temp* thka, QObject* parent)
    : QObject(parent), thka_(thka) {}

void ThkaPoller::start() {
    // This runs in the worker thread (because we connect QThread::started -> start()).
    // Create the timer here so it belongs to the worker thread.
    timer_ = new QTimer(this);
    timer_->setInterval(100); // 10 Hz
    connect(timer_, &QTimer::timeout, this, &ThkaPoller::doPoll);
    timer_->start();
}

void ThkaPoller::queueWrite(int channel, double value) {
    // Called from GUI thread - just queue the request
    QMutexLocker lock(&writeMutex_);
    writeQueue_.push({channel, value});
    qDebug() << "[ThkaPoller] Queued write: CH" << channel << "=" << value << "°C";
}

void ThkaPoller::processWrites() {
    // Process all queued writes (runs in worker thread)
    QMutexLocker lock(&writeMutex_);
    
    while (!writeQueue_.empty()) {
        ThkaWriteRequest req = writeQueue_.front();
        writeQueue_.pop();
        
        // Unlock while doing the actual write (don't block GUI thread from queuing more)
        lock.unlock();
        
        qDebug() << "[ThkaPoller] Processing write: CH" << req.channel << "=" << req.value << "°C";
        
        bool success = false;
        try {
            success = thka_->write_setpoint_celsius(req.channel, req.value);
        } catch (const std::exception& e) {
            qWarning() << "[ThkaPoller] Write failed:" << e.what();
        }
        
        emit writeComplete(req.channel, success);
        
        // Re-lock for next iteration
        lock.relock();
    }
}

void ThkaPoller::doPoll() {
    if (!thka_) return;
    
    // First, process any pending writes
    processWrites();
    
    // Then do the temperature read
    QVariantList out;
    try {
        const auto v = thka_->read_all_channels_celsius(); // blocking, worker thread
        out.reserve(static_cast<int>(v.size()));
        for (double d : v) out.push_back(d);
    } catch (const std::exception& e) {
        qWarning() << "[THKA] poll failed:" << e.what();
    }
    emit polled(out);  // queued to GUI thread
}