#include "IR_device_manager.h"

// Hàm khởi tạo đầy đủ cho IRCommand
IRCommand::IRCommand(const String& device, const String& command, 
                     const std::vector<uint8_t>& data, int freq)
    : deviceName(device), commandName(command), 
      rawData(data), frequency(freq) {}

// Chuyển đổi sang JSON
String IRCommand::toJson() const {
    DynamicJsonDocument doc(1024);
    doc["deviceName"] = deviceName;
    doc["commandName"] = commandName;
    
    JsonArray rawDataArray = doc.createNestedArray("rawData");
    for (uint8_t data : rawData) {
        rawDataArray.add(data);
    }
    
    doc["frequency"] = frequency;
    
    String jsonString;
    serializeJson(doc, jsonString);
    return jsonString;
}

// Tạo từ JSON
IRCommand IRCommand::fromJson(const String& jsonString) {
    DynamicJsonDocument doc(1024);
    DeserializationError error = deserializeJson(doc, jsonString);
    
    if (error) {
        // Trả về một lệnh rỗng nếu không parse được
        return IRCommand();
    }

    IRCommand cmd;
    cmd.deviceName = doc["deviceName"].as<String>();
    cmd.commandName = doc["commandName"].as<String>();
    
    JsonArray rawDataArray = doc["rawData"].as<JsonArray>();
    cmd.rawData.clear();
    for (JsonVariant v : rawDataArray) {
        cmd.rawData.push_back(v.as<uint8_t>());
    }
    
    cmd.frequency = doc["frequency"] | 38; // Mặc định 38kHz nếu không có
    
    return cmd;
}

// Hàm khởi tạo IRDeviceManager
IRDeviceManager::IRDeviceManager() {
    // Khởi tạo EEPROM với kích thước lớn hơn
    EEPROM.begin(EEPROM_SIZE);
    loadCommands();
}

// Lưu lệnh IR mới
bool IRDeviceManager::saveIRCommand(
    const String& deviceName, 
    const String& commandName, 
    uint8_t *rawData, 
    int rawLength, 
    int frequency
) {
    // Kiểm tra nếu đã đạt giới hạn số lệnh
    if (commands.size() >= MAX_COMMANDS) {
        // Nếu đầy, xóa lệnh cũ nhất
        commands.erase(commands.begin());
    }

    // Tạo lệnh mới
    std::vector<uint8_t> dataVector(rawData, rawData + rawLength);
    IRCommand newCommand(deviceName, commandName, dataVector, frequency);

    // Thêm vào danh sách
    commands.push_back(newCommand);

    // Lưu vào EEPROM
    saveCommandsToEEPROM();
    return true;
}

// Tìm kiếm lệnh IR
std::vector<IRCommand> IRDeviceManager::findIRCommands(
    const String& deviceName, 
    const String& commandName
) {
    std::vector<IRCommand> matchedCommands;
    for (auto& cmd : commands) {
        if (cmd.deviceName == deviceName && 
            (commandName.isEmpty() || cmd.commandName == commandName)) {
            matchedCommands.push_back(cmd);
        }
    }
    return matchedCommands;
}

// Xóa tất cả các lệnh
void IRDeviceManager::clearAllCommands() {
    commands.clear();
    saveCommandsToEEPROM();
}

// Xóa các lệnh của một thiết bị cụ thể
void IRDeviceManager::removeDeviceCommands(const String& deviceName) {
    commands.erase(
        std::remove_if(commands.begin(), commands.end(), 
            [&deviceName](const IRCommand& cmd) { 
                return cmd.deviceName == deviceName; 
            }), 
        commands.end()
    );
    saveCommandsToEEPROM();
}

// Trả về tổng số lệnh hiện tại
size_t IRDeviceManager::getCommandCount() const {
    return commands.size();
}

// Lưu các lệnh vào EEPROM sử dụng JSON
void IRDeviceManager::saveCommandsToEEPROM() {
    DynamicJsonDocument doc(EEPROM_SIZE);
    JsonArray cmdArray = doc.createNestedArray("commands");
    
    // Chuyển đổi từng lệnh sang JSON
    for (const auto& cmd : commands) {
        cmdArray.add(cmd.toJson());
    }
    
    // Ghi JSON vào EEPROM
    String jsonString;
    serializeJson(doc, jsonString);
    
    // Kiểm tra kích thước
    if (jsonString.length() > EEPROM_SIZE) {
        Serial.println("Error: Not enough EEPROM space");
        return;
    }
    
    // Ghi từng byte vào EEPROM
    for (size_t i = 0; i < jsonString.length(); i++) {
        EEPROM.write(i, jsonString[i]);
    }
    
    // Đánh dấu kết thúc
    EEPROM.write(jsonString.length(), '\0');
    
    EEPROM.commit();
}

// Tải các lệnh từ EEPROM
void IRDeviceManager::loadCommands() {
    // Đọc JSON từ EEPROM
    String jsonString;
    for (int i = 0; i < EEPROM_SIZE; i++) {
        char c = EEPROM.read(i);
        if (c == '\0') break;
        jsonString += c;
    }
    
    // Parse JSON
    DynamicJsonDocument doc(EEPROM_SIZE);
    DeserializationError error = deserializeJson(doc, jsonString);
    
    if (error) {
        Serial.println("Failed to load commands");
        return;
    }
    
    // Xóa danh sách cũ
    commands.clear();
    
    // Đọc từng lệnh
    JsonArray cmdArray = doc["commands"].as<JsonArray>();
    for (JsonVariant v : cmdArray) {
        String cmdJson = v.as<String>();
        IRCommand cmd = IRCommand::fromJson(cmdJson);
        commands.push_back(cmd);
    }
}