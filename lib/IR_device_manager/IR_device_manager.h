#ifndef IR_DEVICE_MANAGER_H
#define IR_DEVICE_MANAGER_H

#include <Arduino.h>
#include <vector>
#include <EEPROM.h>
#include <ArduinoJson.h>

class IRCommand {
public:
    String deviceName;
    String commandName;
    std::vector<uint8_t> rawData;
    int frequency;

    // Hàm khởi tạo mặc định
    IRCommand() : frequency(38) {}

    // Hàm khởi tạo đầy đủ
    IRCommand(const String& device, const String& command, 
              const std::vector<uint8_t>& data, int freq = 38);

    // Chuyển đổi sang JSON
    String toJson() const;

    // Tạo từ JSON
    static IRCommand fromJson(const String& jsonString);
};

class IRDeviceManager {
private:
    std::vector<IRCommand> commands;
    static const int EEPROM_SIZE = 4096;  // Kích thước EEPROM
    static const int MAX_COMMANDS = 100;  // Số lượng lệnh tối đa

public:
    // Hàm khởi tạo
    IRDeviceManager();

    // Lưu lệnh IR mới
    bool saveIRCommand(
        const String& deviceName, 
        const String& commandName, 
        uint8_t *rawData, 
        int rawLength, 
        int frequency = 38
    );

    // Tìm kiếm lệnh IR
    std::vector<IRCommand> findIRCommands(
        const String& deviceName, 
        const String& commandName = ""
    );

    // Xóa tất cả các lệnh
    void clearAllCommands();

    // Xóa các lệnh của một thiết bị cụ thể
    void removeDeviceCommands(const String& deviceName);

    // Trả về tổng số lệnh hiện tại
    size_t getCommandCount() const;

private:
    // Lưu các lệnh vào EEPROM
    void saveCommandsToEEPROM();

    // Tải các lệnh từ EEPROM
    void loadCommands();
};

#endif // IR_DEVICE_MANAGER_H