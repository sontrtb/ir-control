#include "mqtt_handler.h"
#include <PubSubClient.h>
#include <ArduinoJson.h>
#include "IR_device_manager.h"
#include "IR_handler.h"

// MQTT Broker settings
const char *mqtt_server = "mqtt.eclipseprojects.io";
const int mqtt_port = 1883;
const char *client_id = "sonfe123";
const char *mqtt_topic_subscribe = "sonfedev_ir";

WiFiClient espClient;
PubSubClient mqttClient(espClient);

unsigned long lastReconnectAttempt = 0;

String getPart(String data, int index)
{
  int sepIndex = data.indexOf('/');
  if (sepIndex == -1)
    return data; // Nếu không tìm thấy dấu "/"

  if (index == 0)
  {
    return data.substring(0, sepIndex); // Lấy phần trước "/"
  }
  else
  {
    return data.substring(sepIndex + 1); // Lấy phần sau "/"
  }
}

// Callback function for MQTT messages
void callback(char *topic, byte *payload, unsigned int length)
{
  if (length >= 256)
    return;

  char message[256];

  for (int i = 0; i < length && i < 255; i++)
  {
    message[i] = (char)payload[i];
  }

  // Print MQTT value
  Serial.print("MQTT Value: ");
  Serial.println(message);

  DynamicJsonDocument doc(256);
  DeserializationError error = deserializeJson(doc, message);

  if (error)
  {
    Serial.println("JSON Parsing failed:");
    Serial.println(error.c_str());
    return;
  }

  String type = doc["type"];
  String data = doc["data"];

  Serial.println(type.c_str());
  IRDeviceManager irManager;

  if (type == "All_DEVICES")
  {
    std::vector<IRCommand> tvCommands = irManager.findIRCommands(data);
    for (int i = 0; i < tvCommands.size(); i++)
    {
      Serial.println(tvCommands[i].toJson());
    }
  }
  else if (type == "IR_RECEIVER")
  {
    device = getPart(data, 0);
    fuc = getPart(data, 1);
    isReceiver = true;
    Serial.println(device);
    Serial.println(fuc);
  }
  else if (type == "IR_SEND")
  {
    String deviceSend = getPart(data, 0);
    String fucSend = getPart(data, 1);
    std::vector<IRCommand> commands = irManager.findIRCommands(deviceSend, fucSend);

    for (int i = 0; i < commands.size(); i++)
    {
      std::vector<uint8_t> convertedData(commands[i].rawData.begin(), commands[i].rawData.end());
      irHandler.sendCode(convertedData);
      delay(100);
    }
  }
  else if (type == "CLEAR_ALL_COMMANDS")
  {
    irManager.clearAllCommands();
  }
}

// Non-blocking reconnect
bool tryConnect()
{
  if (mqttClient.connect(client_id))
  {
    mqttClient.subscribe(mqtt_topic_subscribe);
    mqttClient.publish(mqtt_topic_subscribe, "Device connected");
    Serial.printf("MQTT Connected successfully\n");
    return true;
  }
  return false;
}

void setupMQTT()
{
  mqttClient.setServer(mqtt_server, mqtt_port);
  mqttClient.setCallback(callback);

  tryConnect();
}

void handleMQTT()
{
  unsigned long currentMillis = millis();

  if (!mqttClient.connected())
  {
    if (currentMillis - lastReconnectAttempt > 5000)
    {
      lastReconnectAttempt = currentMillis;
      if (tryConnect())
      {
        lastReconnectAttempt = 0;
      }
    }
  }
  else
  {
    mqttClient.loop();
  }
}