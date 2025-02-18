//----------------------------------------------------------------------
// ESP32 C3 temperature sensor test
//----------------------------------------------------------------------
#include "driver/temperature_sensor.h"

// The temp sensor handle is global
temperature_sensor_handle_t temp_handle = NULL;

bool initTempSensor(){
  // Create a sensor for the range -10 to 80C
  temperature_sensor_config_t temp_sensor = {
    .range_min = -10,
    .range_max = 80,
  };
  esp_err_t e = temperature_sensor_install(&temp_sensor, &temp_handle);
  if (e != ESP_OK) {
    Serial.printf("Error: temperature_sensor_install returned %d\n", e);
    temp_handle = NULL;
    return false;
  }
  
  // Enable the sensor
  e = temperature_sensor_enable(temp_handle);
  if (e != ESP_OK) {
    Serial.printf("Error: temperature_sensor_enable returned %d\n", e);
    temperature_sensor_disable(temp_handle);
    return false;
  }

  // Return indicating success
  return true;
}

void setup() {
  Serial.begin(115200);
  initTempSensor();
}

void loop() {
  if (temp_handle) {
    float tsens_out;
    esp_err_t e = temperature_sensor_get_celsius(temp_handle, &tsens_out);
    if (e == ESP_OK) {
      Serial.printf("Temperature = %f\n", tsens_out);
    }
    else {
      Serial.printf("Error: temperature_sensor_enable returned %d\n", e);
      temperature_sensor_disable(temp_handle);
    }
  }

  delay(5000);
}