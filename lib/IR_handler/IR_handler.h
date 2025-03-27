#ifndef IR_HANDLER_H
#define IR_HANDLER_H

#include <Arduino.h>
#include <vector>

class IRHandler {
public:
    IRHandler(int receivePin, int sendPin);
    void begin();
    void sendCode(std::vector<uint16_t> rawData);
    void saveIRData();
    bool decode();

private:
    int receivePin;
    int sendPin;
};

extern IRHandler irHandler;
extern bool isReceiver;
extern String device;
extern String fuc;

#endif
