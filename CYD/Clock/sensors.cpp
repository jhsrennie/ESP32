//----------------------------------------------------------------------
// sensors.cpp
// ===========
// Code to manage the sensors
//----------------------------------------------------------------------
#include <Arduino.h>
#include <Adafruit_AHTX0.h>
#include <Adafruit_BMP280.h>
#include "clock.h"

#define GPIO_SDA 22
#define GPIO_SCL 27
Adafruit_AHTX0 aht;
Adafruit_BMP280 bmp;
bool aht_init = true, bmp_init = true;
float Temperature = 0, Humidity = 0, Pressure = 0;
// If necessary you can use a correction factor to adjust your sensor to
// match the actual pressure readings, but usually the BMP280 is pretty
// accurate and you can just leave this at 100.
float PressureCorrection = 100.0;

//----------------------------------------------------------------------
// InitSensors
// -----------
// Initialise the AHT20 and BMP280
//----------------------------------------------------------------------
void InitSensors() {
  // Set the I2C pins that the CYD uses
  Wire.setPins(GPIO_SDA, GPIO_SCL);
  Wire.begin();

  // Initialise the AHT20
  if (aht.begin()) {
    Serial.println("AHT20 initialised");
    aht_init = true;
  }
  else {
    Serial.println("AHT20 not found");
    aht_init = false;
  }

  if (bmp.begin()) {
    // Default settings from datasheet
    bmp.setSampling(
      Adafruit_BMP280::MODE_NORMAL,     // Operating Mode
      Adafruit_BMP280::SAMPLING_X2,     // Temp. oversampling
      Adafruit_BMP280::SAMPLING_X16,    // Pressure oversampling
      Adafruit_BMP280::FILTER_X16,      // Filtering
      Adafruit_BMP280::STANDBY_MS_500); // Standby time
    Serial.println("BMP280 initialised");
    bmp_init = true;
  }
  else {
    Serial.println("BMP280 not found");
    bmp_init = false;
  }
}

//----------------------------------------------------------------------
// GetTemp
//--------
// read the temperature, etc from the AHT11
//----------------------------------------------------------------------
void GetTemp() {
  // Query the AHT20
  if (aht_init) {
    sensors_event_t humidity, temp;
    aht.getEvent(&humidity, &temp);
    Serial.printf("Temp: %f, humidity: %f\n", temp.temperature, humidity.relative_humidity);

    Temperature = temp.temperature;        // Temp is in Celsius
    Humidity = humidity.relative_humidity; // RH is in %
  }

  // Query the BMP280
  if (bmp_init) {
    float bmp_temp = bmp.readTemperature();  // Temp is in Celsius
    float bmp_pressure = bmp.readPressure(); // Pressure is in Pa
    Serial.printf("Temp: %f, pressure: %f\n", bmp_temp, bmp_pressure);

    Pressure = bmp_pressure/PressureCorrection;
  }
}