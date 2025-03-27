#include "wifiConfig.h"
#include "mqtt_handler.h"
#include "IR_handler.h"
#include "led_control.h"

#define IR_RECEIVE_PIN 15
#define IR_SEND_PIN 23

LedControl ledControl;

void setup() {
    Serial.begin(115200);
    ledControl.begin();

    wifiConfig.begin();
    irHandler.begin();
}

void loop() {
    ledControl.run();
    wifiConfig.run();

    if (WiFi.status() == WL_CONNECTED) {
        handleMQTT();
    }

    if (irHandler.decode() && isReceiver) {
        Serial.println("IR signal received");
        irHandler.saveIRData();
    }
}
