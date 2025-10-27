#pragma once

#include "../ITempSensor.h"
#include <string>
#include <vector>
#include <cstdint>
#include <mutex>  // ADD THIS

struct ThkaChannel {
  int id;
  uint16_t reg_meas;
  uint16_t reg_sv;
  double scale;
};

struct ThkaConfig {
  std::string device = "/dev/ttyUSB0";
  int baud = 9600;
  char parity = 'N';
  int databits = 8;
  int stopbits = 1;
  int slave_id = 1;
  std::vector<ThkaChannel> channels;
};

class ThkaRs485Temp : public ITempSensor {
public:
  explicit ThkaRs485Temp(const ThkaConfig& cfg);
  ~ThkaRs485Temp() override;

  double read_celsius() override;
  double read_channel_celsius(int ch);
  bool   write_setpoint_celsius(int ch, double value);
  std::vector<double> read_all_channels_celsius();

private:
  struct Impl;
  Impl* p_;
  mutable std::mutex modbus_mutex_;  // ADD THIS - protects serial port access
};