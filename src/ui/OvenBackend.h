 #pragma once
 #include <QObject>
 #include <QThread>
 #include <QTimer>
 #include <QVariant>
 #include <QString>
 #include "../core/StateMachine.h"
 
 class ThkaRs485Temp;
 class ThkaPoller;
 
 class OvenBackend : public QObject {
     Q_OBJECT
     Q_PROPERTY(QString status READ status NOTIFY statusChanged)
     Q_PROPERTY(QVariantList thkaTemps READ thkaTemps NOTIFY thkaTempsChanged)
     Q_PROPERTY(double manualSetpoint READ manualSetpoint NOTIFY manualSetpointChanged)
     Q_PROPERTY(QString manualSetpointStatus READ manualSetpointStatus NOTIFY manualSetpointStatusChanged)
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
     Q_INVOKABLE void sendManualSetpoint(double value); // Push a manual target temperature to the THKA controller
 
     QString status() const { return status_; }
     QVariantList thkaTemps() const { return thkaTemps_; }
     double manualSetpoint() const { return manualSetpoint_; }
     QString manualSetpointStatus() const { return manualSetpointStatus_; }
 
 signals:
     void statusChanged();
     void thkaTempsChanged();
     void manualSetpointChanged();
     void manualSetpointStatusChanged();
 
 private slots:
     void onTick();
     void onThkaUpdate(const QVariantList& temps);
 
 private:
     void setStatus(const QString& s);
     void setManualSetpoint(double value);
     void setManualSetpointStatus(const QString& status);
 
     StateMachine* sm_ = nullptr;        // not owned
     ThkaRs485Temp* thka_ = nullptr;     // not owned
 
     QString status_ = "Idle";
     QTimer  tick_;                      // GUI thread tick
 
     // worker thread + poller
     QThread     thkaThread_;
     ThkaPoller* poller_ = nullptr;
 
     // cache for QML
     QVariantList thkaTemps_;
     double manualSetpoint_ = 25.0;           // Cached manual temperature selection (°C)
     QString manualSetpointStatus_ = "THKA controller not connected"; // Human-readable status line
     int manualSetpointChannel_ = 1;          // Channel we write SVs to (adjust if needed)
 };
