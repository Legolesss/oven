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
#include "hw/impl/MockTemp.h"
#include "hw/impl/ThkaRs485Temp.h"
#include "ui/OvenBackend.h"

int main(int argc, char* argv[]) {
  QGuiApplication app(argc, argv);

  // ---- REAL THKA CONFIG ----
  ThkaConfig cfg;
  cfg.channels = {
    {1, 768, 0, 0.1},  // CH1: Read temp from 768, Write setpoint to 0
    {2, 769, 1, 0.1},  // CH2: Read temp from 769, Write setpoint to 1
    {3, 770, 2, 0.1},  // CH3: Read temp from 770, Write setpoint to 2
    {4, 771, 3, 0.1},  // CH4: Read temp from 771, Write setpoint to 3
    {5, 772, 4, 0.1},  // CH5: Read temp from 772, Write setpoint to 4
    {6, 773, 5, 0.1},  // CH6: Read temp from 773, Write setpoint to 5
  };
  
  ThkaRs485Temp thka(cfg);

  // ---- GPIO ----
  constexpr const char* CHIP = "gpiochip0";
  constexpr unsigned GPIO_HEATER = 5;
  constexpr unsigned GPIO_FAN    = 6;
  constexpr bool ACTIVE_HIGH     = false;

  GpioHeater heater(CHIP, GPIO_HEATER, ACTIVE_HIGH);
  GpioFan    fan   (CHIP, GPIO_FAN,    ACTIVE_HIGH);

  // ---- State Machine Params ----
  Params P{};
  P.air_target_c       = 200.0;
  P.air_hysteresis_c   = 5.0;
  P.part_target_c      = 180.0;
  P.part_hysteresis_c  = 3.0;
  P.dwell_seconds      = 20 * 60;
  P.part_min_valid_c   = 120.0;
  P.ir_drop_delta_c    = 15.0;

  MockTemp air_sensor;   air_sensor.inject(25.0);
  MockTemp part_sensor;  part_sensor.inject(25.0);

  StateMachine sm(P, air_sensor, part_sensor, heater, fan);

  // ---- Backend ----
  OvenBackend backend(&sm);
  backend.setThka(&thka);

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
  std::cout << "GUI ready - use touchscreen to control\n" << std::endl;
  
  return app.exec();
}