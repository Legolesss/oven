#pragma once
#include <vector>
#include <chrono>
#include <string>
#include <fstream>
#include <sstream>
#include <iomanip>
#include <ctime>

struct DataPoint {
    std::chrono::steady_clock::time_point timestamp;
    double ch1_temp;  // Air
    double ch2_temp;
    double ch3_temp;
    // ch4 is skipped
    double ch5_temp;
    double ch6_temp;  // IR
    double setpoint;
    std::string state;
};

class DataLogger {
public:
    DataLogger() = default;
    
    void startSession(double setpoint) {
        data_.clear();
        session_setpoint_ = setpoint;
        session_start_ = std::chrono::steady_clock::now();
        logging_active_ = true;
        
        // Generate filename with timestamp
        auto now = std::chrono::system_clock::now();
        auto time_t = std::chrono::system_clock::to_time_t(now);
        std::stringstream ss;
        ss << std::put_time(std::localtime(&time_t), "%Y%m%d_%H%M%S");
        session_filename_ = "cure_log_" + ss.str();
    }
    
    void stopSession() {
        logging_active_ = false;
    }
    
    void logPoint(double ch1, double ch2, double ch3, double ch5, double ch6, 
                  const std::string& state) {
        if (!logging_active_) return;
        
        DataPoint point;
        point.timestamp = std::chrono::steady_clock::now();
        point.ch1_temp = ch1;
        point.ch2_temp = ch2;
        point.ch3_temp = ch3;
        point.ch5_temp = ch5;
        point.ch6_temp = ch6;
        point.setpoint = session_setpoint_;
        point.state = state;
        
        data_.push_back(point);
    }
    
    bool saveToCSV(const std::string& directory = "/home/pi/cure_logs") const {
        if (data_.empty()) return false;
        
        std::string filepath = directory + "/" + session_filename_ + ".csv";
        std::ofstream file(filepath);
        
        if (!file.is_open()) return false;
        
        // Header
        file << "Time(s),CH1_Air(°C),CH2(°C),CH3(°C),CH5(°C),CH6_IR(°C),Setpoint(°C),State\n";
        
        // Data
        for (const auto& point : data_) {
            auto elapsed = std::chrono::duration_cast<std::chrono::milliseconds>(
                point.timestamp - session_start_).count() / 1000.0;
            
            file << std::fixed << std::setprecision(2)
                 << elapsed << ","
                 << point.ch1_temp << ","
                 << point.ch2_temp << ","
                 << point.ch3_temp << ","
                 << point.ch5_temp << ","
                 << point.ch6_temp << ","
                 << point.setpoint << ","
                 << point.state << "\n";
        }
        
        file.close();
        return true;
    }
    
    const std::vector<DataPoint>& getData() const { return data_; }
    std::string getSessionFilename() const { return session_filename_; }
    bool isLogging() const { return logging_active_; }
    
private:
    std::vector<DataPoint> data_;
    double session_setpoint_{0.0};
    std::chrono::steady_clock::time_point session_start_;
    bool logging_active_{false};
    std::string session_filename_;
};