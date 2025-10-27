#pragma once
#include <QObject>
#include <QTimer>
#include <QVariant>

class ThkaRs485Temp;

class ThkaPoller : public QObject {
    Q_OBJECT
public:
    explicit ThkaPoller(ThkaRs485Temp* thka, QObject* parent = nullptr);

public slots:
    void start();  // will be called after moveToThread()

signals:
    void polled(const QVariantList& temps);

private slots:
    void doPoll();

private:
    ThkaRs485Temp* thka_{nullptr};     // not owned
    QTimer* timer_{nullptr};           // construct in start() (worker thread)
};