#include "wifiConfig.h"
#include "mqtt_handler.h"
#include "IR_handler.h"

#define IR_RECEIVE_PIN 15
#define IR_SEND_PIN 23

void setup() {
    Serial.begin(115200);
    wifiConfig.begin();
    irHandler.begin();
}

void loop() {
    wifiConfig.run();

    if (WiFi.status() == WL_CONNECTED) {
        handleMQTT();
    }

    if (irHandler.decode()) {
        Serial.println("IR signal received");
        irHandler.saveIRData();
    }
}
