// *****************************************************
// I've put the EMQX code in a separate file for
// tidyness.
// *****************************************************

#include <WiFi.h>
#include "PubSubClient.h"

// *****************************************************
// These are constants needed for the connection
// *****************************************************

// MQTT Broker
const char *mqtt_broker = "broker.emqx.io";
const char *topic = "foo/bar";
const char *mqtt_username = "emqx";
const char *mqtt_password = "public";
const int mqtt_port = 1883;

// *****************************************************
// This function connects to the EMQX messaging server
// *****************************************************

void connectEMQX(PubSubClient& Client, MQTT_CALLBACK_SIGNATURE) {
  String client_id;
  // Connect to the MQTT server
  Client.setServer(mqtt_broker, mqtt_port);
  Client.setCallback(callback);
  while (!Client.connected()) {
    client_id = "esp32-Client-" + String(WiFi.macAddress());
    Serial.printf("Connecting to %s as %s\n", mqtt_broker, client_id.c_str());
    if (Client.connect(client_id.c_str(), mqtt_username, mqtt_password)) {
      Serial.println("Connected");
    } else {
      Serial.print("failed with state ");
      Serial.println(Client.state());
      delay(2000);
    }
  }
  // Subcribe to the topic so we get messages
  Client.subscribe(topic);
  // Send a test message
  String s = "Hello from " + client_id;
  Client.publish(topic, s.c_str());
}