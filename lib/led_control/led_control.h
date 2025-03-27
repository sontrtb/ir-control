#ifndef LED_CONTROL_H
#define LED_CONTROL_H

#include <Arduino.h>

// Configuration for LED and button pins
#define LED_PIN 2
#define BTN_PIN 0

class LedControl {
public:
    void begin();
    void run();

private:
    void blinkLed(uint32_t interval);

    unsigned long lastTimePress = 0;
    unsigned long blinkTime = 0;
    const unsigned long PUSH_TIME = 5000;
};

extern int wifiMode; // 0:Chế độ cấu hình, 1:Chế độ kết nối, 2: Mất wifi

#endif // LED_CONTROL_H