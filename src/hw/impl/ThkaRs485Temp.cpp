#include "ThkaRs485Temp.h"      // Include this class's header
#include <modbus.h>             // libmodbus C library (handles RS485/RTU protocol)
#include <cmath>                // For std::nan() to represent invalid reads
#include <stdexcept>            // For throwing exceptions on fatal errors
#include <cerrno>               // For errno (error codes from system calls)

// -----------------------------------------------------------------------------
// Internal "Impl" struct hides low-level libmodbus details from the main header.
// This keeps your header clean and decoupled from libmodbus headers.
// -----------------------------------------------------------------------------
struct ThkaRs485Temp::Impl {
  ThkaConfig cfg;               // Stores serial configuration and channel list
  modbus_t* ctx{nullptr};       // libmodbus context pointer (represents an open Modbus connection)

  // -------------------------------------------------------------
  // Constructor: opens and configures the RS485 serial connection
  // -------------------------------------------------------------
  explicit Impl(const ThkaConfig& c) : cfg(c) {
    // Create a new Modbus RTU connection context (device path, baud, parity, data bits, stop bits)
    ctx = modbus_new_rtu(c.device.c_str(), c.baud, c.parity, c.databits, c.stopbits);
    if (!ctx)
      throw std::runtime_error("modbus_new_rtu failed (could not open serial port)");

    // Set the Modbus slave address (must match the "Addr" in the THKA menu)
    if (modbus_set_slave(ctx, c.slave_id) == -1)
      throw std::runtime_error("modbus_set_slave failed");

    // Define the maximum wait time for a response (1 second here)
    timeval tv{1, 0};
    modbus_set_response_timeout(ctx, tv.tv_sec, tv.tv_usec);

    // Attempt to open the connection to the serial port
    if (modbus_connect(ctx) == -1)
      throw std::runtime_error(std::string("modbus_connect failed: ") + modbus_strerror(errno));
  }

  // -------------------------------------------------------------
  // Destructor: closes and frees the Modbus connection cleanly
  // -------------------------------------------------------------
  ~Impl() {
    if (ctx) {
      modbus_close(ctx);  // Close the serial port
      modbus_free(ctx);   // Free all resources allocated by libmodbus
    }
  }

  // -------------------------------------------------------------
  // Reads a single register (input or holding) and returns scaled °C
  // -------------------------------------------------------------
  double read_reg(uint16_t reg, double scale) {
    uint16_t val{};  // Buffer to store 16-bit register value

    // Try to read as an "input register" (function code 0x04)
    int rc = modbus_read_input_registers(ctx, reg, 1, &val);

    // If that fails, try as a "holding register" (function code 0x03)
    if (rc != 1)
      rc = modbus_read_registers(ctx, reg, 1, &val);

    // If both fail, return NaN (invalid read)
    if (rc != 1)
      return std::nan("");

    // Convert raw register value into °C (apply scaling)
    // Example: if val=2500 and scale=0.1 → 250.0°C
    return val * scale;
  }

  // -------------------------------------------------------------
  // Writes a single register (function code 0x06)
  // Converts °C to scaled integer before writing.
  // -------------------------------------------------------------
  bool write_reg(uint16_t reg, double value, double scale) {
    // Convert floating °C to integer (e.g. 200.0°C → 2000 if scale=0.1)
    uint16_t raw = static_cast<uint16_t>(value / scale);

    // Write it to the Modbus holding register
    int rc = modbus_write_register(ctx, reg, raw);

    // Return true if successful (1 register written)
    return rc == 1;
  }
};

// -----------------------------------------------------------------------------
// Public constructor: creates the Impl object and opens connection
// -----------------------------------------------------------------------------
ThkaRs485Temp::ThkaRs485Temp(const ThkaConfig& c)
    : p_(new Impl(c)) {} // Uses new so Impl can hold state safely across this class

// -----------------------------------------------------------------------------
// Destructor: deletes the Impl (which closes the connection automatically)
// -----------------------------------------------------------------------------
ThkaRs485Temp::~ThkaRs485Temp() {
  delete p_;
}

// -----------------------------------------------------------------------------
// Implements ITempSensor::read_celsius()
// Reads the *first* channel defined in config (for backward compatibility)
// -----------------------------------------------------------------------------
double ThkaRs485Temp::read_celsius() {
  if (p_->cfg.channels.empty())
    return std::nan("");  // No channels configured → invalid reading
  return read_channel_celsius(p_->cfg.channels.front().id);  // Read the first channel
}

// -----------------------------------------------------------------------------
// Reads the temperature for a specific channel number (1–6)
// -----------------------------------------------------------------------------
double ThkaRs485Temp::read_channel_celsius(int ch) {
  // Look up the channel in the configuration vector
  for (const auto& c : p_->cfg.channels) {
    if (c.id == ch)
      return p_->read_reg(c.reg_meas, c.scale);  // Read and scale register value
  }
  return std::nan("");  // If channel not found → invalid reading
}

// -----------------------------------------------------------------------------
// Writes a setpoint temperature (°C) to a specific channel
// -----------------------------------------------------------------------------
bool ThkaRs485Temp::write_setpoint_celsius(int ch, double value) {
  // Find matching channel in configuration
  for (const auto& c : p_->cfg.channels) {
    if (c.id == ch)
      return p_->write_reg(c.reg_sv, value, c.scale);  // Write value
  }
  return false;  // Channel not found → failed
}

// -----------------------------------------------------------------------------
// Reads all channels sequentially and returns them in a vector<double>
// -----------------------------------------------------------------------------
std::vector<double> ThkaRs485Temp::read_all_channels_celsius() {
  static std::vector<double> last_valid;                   // store last good values
  const auto& channels = p_->cfg.channels;

  if (last_valid.size() != channels.size())
      last_valid.assign(channels.size(), std::nan(""));

  std::vector<double> out;   // Create a new vector 'out' that will store the most recent temperature readings.
  out.reserve(channels.size());

  for (size_t i = 0; i < channels.size(); ++i) {   // Loop over each configured channel one by one
    double val = p_->read_reg(channels[i].reg_meas, channels[i].scale);

    if (std::isnan(val)) {
      // fallback to previous valid value
      val = last_valid[i];
    } else {
      // store new valid reading
      last_valid[i] = val;
    }

    out.push_back(val);
  }

  return out;  // Return all readings
}
