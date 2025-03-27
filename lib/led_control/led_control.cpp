#include "led_control.h"
#include "IR_handler.h"

int wifiMode = 0;

void LedControl::begin() {
    pinMode(LED_PIN, OUTPUT);
    pinMode(BTN_PIN, INPUT_PULLUP);
    lastTimePress = millis();
}

void LedControl::blinkLed(uint32_t interval) {
    if (millis() - blinkTime > interval) {
        digitalWrite(LED_PIN, !digitalRead(LED_PIN));
        blinkTime = millis();
    }
}

void LedControl::run() {
    if (digitalRead(BTN_PIN) == LOW) {
        if (millis() - lastTimePress < PUSH_TIME) {
            blinkLed(1000);
        } else {
            blinkLed(100);
        }
    } 
    else if (isReceiver)
    {
        blinkLed(500);
    }
    else {
        switch (wifiMode) {
            case 0:
                blinkLed(100);
                break;
            case 1:
                blinkLed(3000);
                break;
            case 2:
                blinkLed(300);
                break;
        }
    }
}