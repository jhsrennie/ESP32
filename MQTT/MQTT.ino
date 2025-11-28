// *****************************************************
// Test of the MQTT communications
//
// Code is from:
// https://www.emqx.com/en/blog/esp32-connects-to-the-free-public-mqtt-broker
//
// The PubSubClient code is from:
// https://wokwi.com/projects/316434364352168512
// *****************************************************

#include <WiFi.h>
#include "PubSubClient.h"
#include "EMQX.h"

// WiFi constants
const char *ssid = "MySSID";
const char *password = "MyPassword";

// Objects needed for the connection
WiFiClient espClient;
PubSubClient client(espClient);

// *****************************************************
// This function connects to the Wowki wireless network
// *****************************************************

void connectWowkiWiFi() {
  WiFi.begin(ssid, password);
  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.println("Connecting to WiFi..");
  }
  Serial.println("Connected to the Wi-Fi network");
  Serial.print("IP address = ");
  Serial.println(WiFi.localIP());
}

// *****************************************************
// Setup
// *****************************************************

void setup() {
  // Set software serial baud to 115200;
  Serial.begin(115200);

  // Connecting to the WiFi network
  connectWowkiWiFi();

  // Connecting to EQMX
  connectEMQX(client, callback);

  // We'll control the LED on pin 12
  #define PIN_LED 12
  pinMode(PIN_LED, OUTPUT);
  digitalWrite(PIN_LED, LOW);

  // And we'll read the voltage on pin 34
  #define PIN_POT 34
  pinMode(PIN_POT, INPUT);
}

// *****************************************************
// The loop() just calls the message object loop function
// *****************************************************

void loop() {
    client.loop();
}

// *****************************************************
// This function gets called whenever we send the ESP32
// a message, so this is where the hard work is done.
// *****************************************************

void callback(char *topic, byte *payload, unsigned int length) {
  Serial.printf("Received message length %d: ", length);
  // Convert the data received to a string
  String msg = "";
  for (int i = 0; i < length; i++)
    msg += (char) payload[i];
  Serial.println(msg);

  // Check the message
  if (msg == "on") {
    Serial.println("Turning LED on");
    digitalWrite(PIN_LED, HIGH);
  }
  else if (msg == "off") {
    Serial.println("Turning LED off");
    digitalWrite(PIN_LED, LOW);
  }
  else if (msg == "read") {
    int x = analogRead(PIN_POT);
    String s = String(x);
    Serial.printf("Pot voltage is %s\n", s);
    client.publish("tejas/test", s.c_str());
  }
}