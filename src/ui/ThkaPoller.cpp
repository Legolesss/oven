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

void ThkaPoller::doPoll() {
    if (!thka_) return;
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
