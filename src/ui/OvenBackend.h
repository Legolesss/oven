#pragma once
#include <QObject>
#include <QThread>
#include <QTimer>
#include <QVariant>
#include "../core/StateMachine.h"

class ThkaRs485Temp;
class ThkaPoller;

class OvenBackend : public QObject {
    Q_OBJECT
    Q_PROPERTY(QString status READ status NOTIFY statusChanged)
    Q_PROPERTY(QVariantList thkaTemps READ thkaTemps NOTIFY thkaTempsChanged)
public:
    explicit OvenBackend(StateMachine* sm, QObject* parent = nullptr);

    // called from main.cpp after you construct ThkaRs485Temp
    void setThka(ThkaRs485Temp* thka);

    Q_INVOKABLE void enterIdle();
    Q_INVOKABLE void enterWarming();
    Q_INVOKABLE void enterReady();
    Q_INVOKABLE void enterCuring();
    Q_INVOKABLE void enterShutdown();
    Q_INVOKABLE void enterFault();

    QString status() const { return status_; }
    QVariantList thkaTemps() const { return thkaTemps_; }

signals:
    void statusChanged();
    void thkaTempsChanged();

private slots:
    void onTick();
    void onThkaUpdate(const QVariantList& temps);

private:
    void setStatus(const QString& s);

    StateMachine* sm_ = nullptr;        // not owned
    ThkaRs485Temp* thka_ = nullptr;     // not owned

    QString status_ = "Idle";
    QTimer  tick_;                      // GUI thread tick

    // worker thread + poller
    QThread     thkaThread_;
    ThkaPoller* poller_ = nullptr;

    // cache for QML
    QVariantList thkaTemps_;
};
