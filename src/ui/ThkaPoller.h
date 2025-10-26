#pragma once
#include <QObject>
#include <QTimer>
#include <QVariant>

class ThkaRs485Temp;

class ThkaPoller : public QObject {
    Q_OBJECT
public:
    explicit ThkaPoller(ThkaRs485Temp* thka, QObject* parent = nullptr);
    std::vector<uint16_t> read_input_registers(uint16_t start_reg, int count);
    std::vector<uint16_t> read_holding_registers(uint16_t start_reg, int count);
    bool write_register_raw(uint16_t reg, uint16_t value);


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
