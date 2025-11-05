#include <QGuiApplication>
#include <QQmlApplicationEngine>
#include <QQmlContext>
#include <iostream>
#include "core/StateMachine.h"
#include "hw/IHeater.h"
#include "hw/IFan.h"
#include "hw/ITempSensor.h"
#include "hw/impl/GpioHeater.h"
#include "hw/impl/GpioFan.h"
#include "hw/impl/GpioRelay.h"
#include "hw/impl/ThkaRs485Temp.h"
#include "hw/impl/ThkaTempAdapter.h"
#include "ui/OvenBackend.h"

int main(int argc, char* argv[]) {
  QGuiApplication app(argc, argv);

  // ---- REAL THKA CONFIG ----
  ThkaConfig cfg;
  cfg.channels = {
    {1, 768, 0, 0.1},  // CH1: Air temp - Read from 768, Write setpoint to 0
    {2, 769, 1, 0.1},  // CH2: Read temp from 769, Write setpoint to 1
    {3, 770, 2, 0.1},  // CH3: Read temp from 770, Write setpoint to 2
    {4, 771, 3, 0.1},  // CH4: Read temp from 771, Write setpoint to 3
    {5, 772, 4, 0.1},  // CH5: Read temp from 772, Write setpoint to 4
    {6, 773, 5, 0.1},  // CH6: IR sensor - Read from 773, Write setpoint to 5
  };
  
  ThkaRs485Temp thka(cfg);

  // ---- NON-BLOCKING TEMPERATURE SENSORS ----
  // These adapters cache the last value from ThkaPoller
  // StateMachine reads from cache (instant, non-blocking)
  // ThkaPoller updates cache in background thread
  ThkaTempAdapter air_sensor(1);   // Channel 1 = air temp
  ThkaTempAdapter part_sensor(6);  // Channel 6 = IR sensor

  std::cout << "\n=== Temperature Sensors Configuration ===" << std::endl;
  std::cout << "Air sensor:  THKA Channel 1 (register 768) - CACHED" << std::endl;
  std::cout << "IR sensor:   THKA Channel 6 (register 773) - CACHED" << std::endl;
  std::cout << "Sensors use non-blocking cached values" << std::endl;

  // ---- GPIO ----
  constexpr const char* CHIP = "gpiochip0";
  constexpr unsigned GPIO_HEATER = 5;
  constexpr unsigned GPIO_FAN    = 6;
  constexpr bool ACTIVE_HIGH     = false;

  GpioRelay fan2 (CHIP, GPIO_HEATER, ACTIVE_HIGH);
  GpioRelay   fan   (CHIP, GPIO_FAN,    ACTIVE_HIGH);
  GpioRelay greenL(CHIP, 13, false);
  GpioRelay amberL(CHIP, 16, false);
  GpioRelay redL  (CHIP, 19, false);
  GpioRelay buzzerL(CHIP, 20, false);
  GpioRelay contactor(CHIP, 26, false);
  
  // ---- State Machine Params ----
  Params P{};
  P.air_target_c       = 200.0;
  P.air_hysteresis_c   = 5.0;
  P.part_target_c      = 180.0;
  P.part_hysteresis_c  = 3.0;
  P.dwell_seconds      = 20 * 60;
  P.part_min_valid_c   = 120.0;
  P.ir_drop_delta_c    = 15.0;

  StateMachine sm(P, air_sensor, part_sensor, fan2, fan, greenL, redL, amberL, buzzerL, contactor);

  // ---- Backend ----
  OvenBackend backend(&sm);
  backend.setThka(&thka);
  backend.setSensorAdapters(&air_sensor, &part_sensor);  // Connect adapters to backend

  // ---- QML Engine ----
  QQmlApplicationEngine engine;
  engine.addImportPath(QStringLiteral("/usr/lib/aarch64-linux-gnu/qt6/qml"));
  engine.rootContext()->setContextProperty("oven", &backend);
  engine.load(QUrl(QStringLiteral("qrc:/OVEN/qml/Main.qml")));
  
  if (engine.rootObjects().isEmpty()) {
    std::cerr << "Failed to load QML!" << std::endl;
    return -1;
  }

  std::cout << "\n=== Oven Controller Started ===" << std::endl;
  std::cout << "THKA registers configured:" << std::endl;
  std::cout << "  Temperature reads: 768-773 (CH1-CH6)" << std::endl;
  std::cout << "  Setpoint writes:   0-5 (CH1-CH6)" << std::endl;
  std::cout << "\nTemperature monitoring:" << std::endl;
  std::cout << "  Air:  CH1 (reg 768) - non-blocking cache" << std::endl;
  std::cout << "  IR:   CH6 (reg 773) - non-blocking cache" << std::endl;
  std::cout << "\nGUI ready - use touchscreen to control\n" << std::endl;
  
  return app.exec();
}