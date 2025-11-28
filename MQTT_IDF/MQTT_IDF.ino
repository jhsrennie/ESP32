// ---------------------------------------------------------------------
// MQTT test
// ---------
// Demonstrate how to use the IDF MQTT API from the Arduino interface
// ---------------------------------------------------------------------
#include <WiFi.h>
#include <mqtt_client.h>

#define LED_BUILTIN 2

// WiFi credentials
String SSID = "MySSID";
String PWD  = "MyPassword";

// MQTT constants
#define CONFIG_BROKER_URL "mqtt://emqx:public@broker.emqx.io:1883"
#define MQTT_TOPIC        "rattus/test"

// MQTT Client handle
esp_mqtt_client_handle_t MQTTClient;

// ---------------------------------------------------------------------
// ConnectWiFi
// ---------------------------------------------------------------------
void ConnectWiFi() {
  WiFi.begin(SSID, PWD);
  int loopcnt = 0;
  while (WiFi.status() != WL_CONNECTED) {
    Serial.printf("Connecting: time %d, WiFi status = %d, signal = %d\n", loopcnt++, WiFi.status(), WiFi.RSSI());
    delay(1000);
  }
  Serial.printf("Connected: %s\n", WiFi.localIP().toString().c_str());
}

// ---------------------------------------------------------------------
// MQTT event handler
// ---------------------------------------------------------------------
void mqtt_event_handler(void *handler_args, esp_event_base_t base, int32_t event_id, void *event_data)
{
  esp_mqtt_event_handle_t event = (esp_mqtt_event_handle_t) event_data;
  int msg_id;

  switch ((esp_mqtt_event_id_t) event_id) {
    // Connected to MQTT server
    case MQTT_EVENT_CONNECTED:
      onConnected(event);
      break;

    // Subscribed to topic
    case MQTT_EVENT_SUBSCRIBED:
        onSubscribed(event);
        break;

    // Received data
    case MQTT_EVENT_DATA:
      onData(event);
      break;

    // Unhandled events end up here
    default:
      Serial.printf("Event: base=%s, event_id=%d\n", base, event_id);
      break;
  }
}

// ---------------------------------------------------------------------
// onConnected
// -----------
// This function is called when the program connects to the MQTT broker
// ---------------------------------------------------------------------
void onConnected(esp_mqtt_event_handle_t event) {
  // When we connect subscribe to the test topic
  esp_mqtt_client_handle_t client = event->client;
  int msg_id = esp_mqtt_client_subscribe(client, MQTT_TOPIC, 0);
  Serial.printf("Subscribe: msg_id=%d\n", msg_id);
}

// ---------------------------------------------------------------------
// onSubscribed
// ------------
// This function is called when the program has subscribed to a topic
// ---------------------------------------------------------------------
void onSubscribed(esp_mqtt_event_handle_t event) {
  // Publish a message in this topic to say we are here
  esp_mqtt_client_handle_t client = event->client;
  int msg_id = esp_mqtt_client_publish(client, MQTT_TOPIC, "hello", 0, 0, 0);
  Serial.printf("Publish: msg_id=%d\n", msg_id);
}

// ---------------------------------------------------------------------
// onData
// ------
// This function is called when data is received in a subscribed topic
// ---------------------------------------------------------------------
void onData(esp_mqtt_event_handle_t event) {
  esp_mqtt_client_handle_t client = event->client;

  // Convert the chars in the arrays event->topic and event->data to strings
  String topic(event->topic, event->topic_len);
  String data(event->data, event->data_len);
  Serial.printf("Received data in topic %s: %s\n", topic.c_str(), data.c_str());

  // The "on" command turns on the built in LED
  if (data == "on")
    digitalWrite(LED_BUILTIN, HIGH);
  // The "off" command turns off the built in LED
  else if (data == "off")
    digitalWrite(LED_BUILTIN, LOW);
}  

// ---------------------------------------------------------------------
// mqtt_app_start
// --------------
// Configure and start the MQTT connection
// ---------------------------------------------------------------------
void mqtt_app_start(void)
{
  esp_mqtt_client_config_t mqtt_cfg;
  memset(&mqtt_cfg, 0, sizeof(mqtt_cfg));

  // The credentials are embedded in the URI
  mqtt_cfg.broker.address.uri = CONFIG_BROKER_URL;
  esp_mqtt_client_handle_t client = esp_mqtt_client_init(&mqtt_cfg);
  if (!client) {
    Serial.println("Connection failed");
  }

  // Set out event handle and start the MTQQ client
  esp_mqtt_client_register_event(client, (esp_mqtt_event_id_t) ESP_EVENT_ANY_ID, mqtt_event_handler, NULL);
  esp_mqtt_client_start(client);

  // Save the handle as a global variable so we can use it from
  // other functions
  MQTTClient = client;
}

// ---------------------------------------------------------------------
// setup
// ---------------------------------------------------------------------
void setup() {
  Serial.begin(115200);
  while (!Serial)
    delay(1000);

  // Connect the WiFi
  Serial.println("Connecting wifi ...");
  ConnectWiFi();

  // We will use the built in LED for testing
  pinMode(LED_BUILTIN, OUTPUT);

  // It appears we don't need an event loop for MQTT
  // I suspect the WiFi client has already created one
  // esp_event_loop_create_default();

  // Start MQTT
  mqtt_app_start();
}

void loop() {
  // Read a string from the serial port
  String s = Serial.readStringUntil(10);
  // Send the string using MQTT
  if (s.length() > 0) {
    esp_mqtt_client_publish(MQTTClient, MQTT_TOPIC, s.c_str(), 0, 0, 0);
  }
}