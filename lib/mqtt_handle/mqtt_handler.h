#ifndef MQTT_HANDLER_H
#define MQTT_HANDLER_H

#include <PubSubClient.h>
#include <WiFi.h>

// MQTT Broker settings
extern const char* mqtt_server;
extern const int mqtt_port;
extern const char* client_id;
extern const char* mqtt_topic_subscribe;

extern PubSubClient mqttClient;

void callback(char* topic, byte* payload, unsigned int length);
void setupMQTT();
void handleMQTT();

#endif // MQTT_HANDLER_H