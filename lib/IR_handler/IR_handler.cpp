#include "IR_handler.h"
#include "IR_device_manager.h"
#include <IRremote.hpp>

#define IR_RECEIVE_PIN 15
#define IR_SEND_PIN 23

bool isReceiver = false;
String device = "";
String fuc = "";

IRHandler irHandler(IR_RECEIVE_PIN, IR_SEND_PIN);

IRHandler::IRHandler(int receivePin, int sendPin)
    : receivePin(receivePin), sendPin(sendPin) {}

void IRHandler::begin()
{
    IrReceiver.begin(receivePin);
    IrSender.begin(sendPin);
}

bool IRHandler::decode()
{
    if (IrReceiver.decode())
    {
        IrReceiver.resume();
        return true;
    }
    return false;
}

void IRHandler::sendCode(std::vector<uint16_t> rawData)
{
    IrSender.sendRaw(rawData.data(), rawData.size(), 38);
    Serial.print(F("raw "));
    Serial.print(rawData.size());
    Serial.println(F(" marks or spaces"));
}

void IRHandler::saveIRData()
{
    IRDeviceManager irManager;
    if (IrReceiver.decodedIRData.protocol == UNKNOWN)
    {
        Serial.println("Unknown Protocol - Raw Data:");
        int rawCodeLength = IrReceiver.decodedIRData.rawDataPtr->rawlen - 1;
        uint8_t *rawCode = new uint8_t[rawCodeLength];
        IrReceiver.compensateAndStoreIRResultInArray(rawCode);

        Serial.print("rawCodeLength: ");
        Serial.println(rawCodeLength);

        if (rawCodeLength < 4)
            return;

        Serial.println("Saving...");
        bool saveStatus = irManager.saveIRCommand(device, fuc, rawCode, rawCodeLength, 38);
        Serial.println(saveStatus ? "Save SUCCESS" : "Save ERROR");

        delete[] rawCode;
    }
    else
    {
        Serial.println("Decoded IR Signal:");
        Serial.print("Protocol: ");
        Serial.println(IrReceiver.decodedIRData.protocol);
        Serial.print("Address: 0x");
        Serial.println(IrReceiver.decodedIRData.address, HEX);
        Serial.print("Command: 0x");
        Serial.println(IrReceiver.decodedIRData.command, HEX);
        Serial.println("Flags:");
        if (IrReceiver.decodedIRData.flags & IRDATA_FLAGS_IS_REPEAT)
            Serial.println("- Repeat Signal");
        if (IrReceiver.decodedIRData.flags & IRDATA_FLAGS_IS_AUTO_REPEAT)
            Serial.println("- Auto Repeat");
        if (IrReceiver.decodedIRData.flags & IRDATA_FLAGS_PARITY_FAILED)
            Serial.println("- Parity Error");
    }
    isReceiver = false;
}