#pragma once
#include <QObject>
#include <QThread>
#include <QTimer>
#include <QVariant>
#include <QString>
#include "../core/StateMachine.h"

class ThkaRs485Temp;
class ThkaPoller;
class ThkaTempAdapter;

class OvenBackend : public QObject {
    Q_OBJECT
    Q_PROPERTY(QString status READ status NOTIFY statusChanged)
    Q_PROPERTY(QVariantList thkaTemps READ thkaTemps NOTIFY thkaTempsChanged)
    Q_PROPERTY(double manualSetpoint READ manualSetpoint NOTIFY manualSetpointChanged)
    Q_PROPERTY(QString manualSetpointStatus READ manualSetpointStatus NOTIFY manualSetpointStatusChanged)
    
    // Auto mode properties
    Q_PROPERTY(bool autoModeActive READ autoModeActive NOTIFY autoModeActiveChanged)
    Q_PROPERTY(double autoTargetTemp READ autoTargetTemp NOTIFY autoTargetTempChanged)
    Q_PROPERTY(QString autoStatus READ autoStatus NOTIFY autoStatusChanged)
    Q_PROPERTY(int autoCureTimeLeft READ autoCureTimeLeft NOTIFY autoCureTimeLeftChanged)
    Q_PROPERTY(bool autoCureComplete READ autoCureComplete NOTIFY autoCureCompleteChanged)

public:
    explicit OvenBackend(StateMachine* sm, QObject* parent = nullptr);

    void setThka(ThkaRs485Temp* thka);
    void setSensorAdapters(ThkaTempAdapter* air, ThkaTempAdapter* part);

    // Manual mode commands
    Q_INVOKABLE void enterIdle();
    Q_INVOKABLE void enterWarming();
    Q_INVOKABLE void enterReady();
    Q_INVOKABLE void enterCuring();
    Q_INVOKABLE void enterShutdown();
    Q_INVOKABLE void enterFault();
    Q_INVOKABLE void sendManualSetpoint(double value);

    // Auto mode commands
    Q_INVOKABLE void startAutoMode(double targetTemp);
    Q_INVOKABLE void cancelAutoMode();
    Q_INVOKABLE void acknowledgeAutoCureComplete();

    // Getters
    QString status() const { return status_; }
    QVariantList thkaTemps() const { return thkaTemps_; }
    double manualSetpoint() const { return manualSetpoint_; }
    QString manualSetpointStatus() const { return manualSetpointStatus_; }
    
    bool autoModeActive() const { return autoModeActive_; }
    double autoTargetTemp() const { return autoTargetTemp_; }
    QString autoStatus() const { return autoStatus_; }
    int autoCureTimeLeft() const { return autoCureTimeLeft_; }
    bool autoCureComplete() const { return autoCureComplete_; }

signals:
    void statusChanged();
    void thkaTempsChanged();
    void manualSetpointChanged();
    void manualSetpointStatusChanged();
    
    void autoModeActiveChanged();
    void autoTargetTempChanged();
    void autoStatusChanged();
    void autoCureTimeLeftChanged();
    void autoCureCompleteChanged();

private slots:
    void onTick();
    void onThkaUpdate(const QVariantList& temps);
    void onWriteComplete(int channel, bool success);

private:
    void setStatus(const QString& s);
    void setManualSetpoint(double value);
    void setManualSetpointStatus(const QString& status);
    
    void setAutoStatus(const QString& status);
    void setAutoCureTimeLeft(int seconds);
    void setAutoCureComplete(bool complete);
    
    void updateAutoModeStatus();

    StateMachine* sm_ = nullptr;
    ThkaRs485Temp* thka_ = nullptr;

    QString status_ = "Idle";
    QTimer  tick_;

    QThread     thkaThread_;
    ThkaPoller* poller_ = nullptr;
    
    // Sensor adapters (not owned)
    ThkaTempAdapter* airAdapter_ = nullptr;
    ThkaTempAdapter* partAdapter_ = nullptr;

    QVariantList thkaTemps_;
    double manualSetpoint_ = 25.0;
    QString manualSetpointStatus_ = "THKA controller not connected";
    int manualSetpointChannel_ = 1;

    // Auto mode state (mirrors StateMachine state)
    bool autoModeActive_ = false;
    double autoTargetTemp_ = 200.0;
    QString autoStatus_ = "Not Active";
    int autoCureTimeLeft_ = 0;
    bool autoCureComplete_ = false;
};