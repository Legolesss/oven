#include "ThkaRs485Temp.h"
#include <modbus/modbus.h>
#include <cmath>
#include <stdexcept>
#include <cerrno>
#include <iostream>
#include <mutex>
#include <thread>
#include <chrono>

// -----------------------------------------------------------------------------
// Internal "Impl" struct - MUST be defined BEFORE we use p_->anything
// -----------------------------------------------------------------------------
struct ThkaRs485Temp::Impl {
  ThkaConfig cfg;
  modbus_t* ctx{nullptr};

  explicit Impl(const ThkaConfig& c) : cfg(c) {
    ctx = modbus_new_rtu(c.device.c_str(), c.baud, c.parity, c.databits, c.stopbits);
    if (!ctx)
      throw std::runtime_error("modbus_new_rtu failed (could not open serial port)");

    if (modbus_set_slave(ctx, c.slave_id) == -1)
      throw std::runtime_error("modbus_set_slave failed");

    timeval tv{1, 0};
    modbus_set_response_timeout(ctx, tv.tv_sec, tv.tv_usec);

    if (modbus_connect(ctx) == -1)
      throw std::runtime_error(std::string("modbus_connect failed: ") + modbus_strerror(errno));
  }

  ~Impl() {
    if (ctx) {
      modbus_close(ctx);
      modbus_free(ctx);
    }
  }

  double read_reg(uint16_t reg, double scale) {
    uint16_t val{};
    int rc = modbus_read_input_registers(ctx, reg, 1, &val);
    if (rc != 1)
      rc = modbus_read_registers(ctx, reg, 1, &val);
    if (rc != 1)
      return std::nan("");
    return val * scale;
  }

  bool write_reg(uint16_t reg, double value, double scale) {
    uint16_t raw = static_cast<uint16_t>(value / scale);
    int rc = modbus_write_register(ctx, reg, raw);
    return rc == 1;
  }
};

// -----------------------------------------------------------------------------
// Now the public methods (AFTER Impl is fully defined)
// -----------------------------------------------------------------------------

ThkaRs485Temp::ThkaRs485Temp(const ThkaConfig& c)
    : p_(new Impl(c)) {}

ThkaRs485Temp::~ThkaRs485Temp() {
  delete p_;
}

double ThkaRs485Temp::read_celsius() {
  if (p_->cfg.channels.empty())
    return std::nan("");
  return read_channel_celsius(p_->cfg.channels.front().id);
}

double ThkaRs485Temp::read_channel_celsius(int ch) {
  std::lock_guard<std::mutex> lock(modbus_mutex_);  // Thread-safe
  
  for (const auto& c : p_->cfg.channels) {
    if (c.id == ch)
      return p_->read_reg(c.reg_meas, c.scale);
  }
  return std::nan("");
}

bool ThkaRs485Temp::write_setpoint_celsius(int ch, double value) {
  std::lock_guard<std::mutex> lock(modbus_mutex_);  // Thread-safe - CRITICAL!
  
  for (const auto& c : p_->cfg.channels) {
    if (c.id == ch) {
      uint16_t raw = static_cast<uint16_t>(value / c.scale);
      
      std::cout << "[THKA] Writing " << value << "°C to CH" << ch 
                << " (register " << c.reg_sv << ", raw value " << raw << ")" << std::endl;
      
      // Small delay to ensure bus is clear
      std::this_thread::sleep_for(std::chrono::milliseconds(50));
      
      bool modbus_result = p_->write_reg(c.reg_sv, value, c.scale);
      
      // QUIRK: THKA registers 0-5 report failure but actually work
      if (c.reg_sv >= 0 && c.reg_sv <= 5) {
        std::cout << "[THKA] Write sent to setpoint register " << c.reg_sv 
                  << " (registers 0-5 report false failure)" << std::endl;
        
        // Give THKA time to process
        std::this_thread::sleep_for(std::chrono::milliseconds(100));
        
        return true;  // Always succeed for 0-5
      }
      
      if (modbus_result) {
        std::cout << "[THKA] Write successful!" << std::endl;
      } else {
        std::cerr << "[THKA] Write failed to register " << c.reg_sv << std::endl;
      }
      
      return modbus_result;
    }
  }
  
  std::cerr << "[THKA] ERROR: Channel " << ch << " not in config!" << std::endl;
  return false;
}

std::vector<double> ThkaRs485Temp::read_all_channels_celsius() {
  std::lock_guard<std::mutex> lock(modbus_mutex_);  // Thread-safe
  
  static std::vector<double> last_valid;
  const auto& channels = p_->cfg.channels;

  if (last_valid.size() != channels.size())
      last_valid.assign(channels.size(), std::nan(""));

  std::vector<double> out;
  out.reserve(channels.size());

  for (size_t i = 0; i < channels.size(); ++i) {
    double val = p_->read_reg(channels[i].reg_meas, channels[i].scale);

    if (std::isnan(val)) {
      val = last_valid[i];
    } else {
      last_valid[i] = val;
    }

    out.push_back(val);
  }

  return out;
}