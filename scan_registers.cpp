#include <iostream>
#include <cmath>
#include "hw/impl/ThkaRs485Temp.h"

int main() {
    std::cout << "Testing Registers 0-5 (Manual says '1-6 channel setting value')\n" << std::endl;
    
    ThkaConfig cfg;
    cfg.device = "/dev/ttyUSB0";
    cfg.baud = 9600;
    cfg.parity = 'N';
    cfg.databits = 8;
    cfg.stopbits = 1;
    cfg.slave_id = 1;
    
    // First, try to READ registers 0-5 to see what's there
    std::cout << "Step 1: Reading current values in registers 0-5..." << std::endl;
    for (uint16_t reg = 0; reg <= 5; reg++) {
        cfg.channels = {{1, reg, 0, 0.1}};
        try {
            ThkaRs485Temp sensor(cfg);
            double value = sensor.read_channel_celsius(1);
            if (!std::isnan(value)) {
                std::cout << "  Register " << reg << ": " << value << "°C" << std::endl;
            } else {
                std::cout << "  Register " << reg << ": Could not read" << std::endl;
            }
        } catch (...) {
            std::cout << "  Register " << reg << ": Error reading" << std::endl;
        }
    }
    
    std::cout << "\nStep 2: Testing WRITE to register 0 (should be CH1 setpoint)..." << std::endl;
    std::cout << "Current CH1 setpoint on display: _____ °C" << std::endl;
    std::cout << "Press Enter to write 155.5°C to register 0..." << std::endl;
    std::cin.ignore();
    std::cin.get();
    
    cfg.channels = {{1, 768, 0, 0.1}};  // Read from 768, write to 0
    try {
        ThkaRs485Temp sensor(cfg);
        bool success = sensor.write_setpoint_celsius(1, 100);
        
        if (success) {
            std::cout << "Write reported SUCCESS!" << std::endl;
        } else {
            std::cout << "Write reported FAILURE" << std::endl;
        }
        
        std::cout << "\nCheck THKA display - did CH1 setpoint change to 155.5°C?" << std::endl;
        std::cout << "(y/n): ";
        char resp;
        std::cin >> resp;
        
        if (resp == 'y' || resp == 'Y') {
            std::cout << "\n*** SUCCESS! Register 0 is CH1 setpoint! ***\n" << std::endl;
            std::cout << "Configuration:\n";
            std::cout << "cfg.channels = {\n";
            for (int ch = 1; ch <= 6; ch++) {
                std::cout << "  {" << ch << ", " << (767 + ch) << ", " << (ch - 1) << ", 0.1},\n";
            }
            std::cout << "};" << std::endl;
        }
        
    } catch (const std::exception& e) {
        std::cout << "Exception: " << e.what() << std::endl;
    }
    
    return 0;
}
