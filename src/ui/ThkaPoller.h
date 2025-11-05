#pragma once
#include <QObject>
#include <QTimer>
#include <QVariant>
#include <QMutex>
#include <queue>

class ThkaRs485Temp;

struct ThkaWriteRequest {
    int channel;
    double value;
};

class ThkaPoller : public QObject {
    Q_OBJECT
public:
    explicit ThkaPoller(ThkaRs485Temp* thka, QObject* parent = nullptr);

public slots:
    void start();  // will be called after moveToThread()
    void queueWrite(int channel, double value);  // NEW: Queue a write from GUI thread

signals:
    void polled(const QVariantList& temps);
    void writeComplete(int channel, bool success);  // NEW: Signal when write finishes

private slots:
    void doPoll();

private:
    void processWrites();  // NEW: Process queued writes

    ThkaRs485Temp* thka_{nullptr};     // not owned
    QTimer* timer_{nullptr};           // construct in start() (worker thread)
    
    // Thread-safe write queue
    QMutex writeMutex_;
    std::queue<ThkaWriteRequest> writeQueue_;
};