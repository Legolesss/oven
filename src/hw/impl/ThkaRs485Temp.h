#pragma once                     // Prevents the file from being included twice by the compiler

#include "../ITempSensor.h"       // Include your temperature interface definition
#include <string>                 // For std::string (device path, etc.)
#include <vector>                 // For holding multiple channels in std::vector
#include <cstdint>   // <-- add this, gives uint16_t, int16_t, etc.


// -------------------------------------------------------------
// Represents one temperature input channel (RTD or 4–20 mA).
// Each channel has its own Modbus register addresses and scaling.
// -------------------------------------------------------------
struct ThkaChannel {
  int id;           // Channel number (1–6 on your THKA controller)
  uint16_t reg_meas; // The Modbus register address for the measured value
  uint16_t reg_sv;   // The Modbus register address for the setpoint (SV)
  double scale;      // Scaling factor (0.1 = 0.1°C per unit, 1.0 = 1°C per unit)
};

// -------------------------------------------------------------
// Global configuration for the THKA RS485 Modbus connection.
// This defines serial settings and what channels exist.
// -------------------------------------------------------------
struct ThkaConfig {
  std::string device = "/dev/ttyUSB0"; // Serial device (USB-RS485 adapter)
  int baud = 9600;                     // Baud rate (must match controller's "bAud" setting)
  char parity = 'N';                   // Parity (N = none)
  int databits = 8;                    // Data bits per frame (8 typical for Modbus)
  int stopbits = 1;                    // Stop bits (1 typical)
  int slave_id = 1;                    // Modbus address of your THKA (controller parameter "Addr")

  // List of all channels (1–6). Each entry holds register mappings for that channel.
  std::vector<ThkaChannel> channels;
};

// -------------------------------------------------------------
// The main RS485 Modbus temperature sensor class.
// This class implements ITempSensor, so it can replace your mock easily.
// -------------------------------------------------------------
class ThkaRs485Temp : public ITempSensor {
public:
  explicit ThkaRs485Temp(const ThkaConfig& cfg); // Constructor (creates connection)
  ~ThkaRs485Temp() override;                     // Destructor (closes connection)

  // Implements ITempSensor interface (required function)
  double read_celsius() override;                // Reads the temperature of the first configured channel

  // Extended functions specific to THKA
  double read_channel_celsius(int ch);           // Reads a specific channel (CH1–CH6)
  bool   write_setpoint_celsius(int ch, double value); // Writes a temperature setpoint to a channel
  std::vector<double> read_all_channels_celsius();     // Reads all channel temperatures at once

private:
  struct Impl;       // Forward declaration of the internal implementation struct
  Impl* p_;          // Pointer to actual implementation (PImpl pattern, hides libmodbus details)
};
