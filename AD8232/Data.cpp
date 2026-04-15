//----------------------------------------------------------------------
// Data.cpp
// ========
// Data collection module.
// The data collection runs as a separate task. It collects data
// continuously into a circular buffer.
//----------------------------------------------------------------------
#include <Arduino.h>
#include <freertos/FreeRTOS.h>
#include <stdint.h>
#include "Data.h"

// Sensor pins
#define PIN_ADC     8
#define PIN_LOMINUS 9
#define PIN_LOPLUS 10

// Global storage for data
static int IdxADCData;
static uint16_t ADCData[MAX_ADCDATA] = {0};

// Used to block data collection while reading the data buffer.
// Effectively a mutex but a cut price one.
static bool client_reading_data = false;

// Task handle
#define DATATASK_STACK 2000
static TaskHandle_t hDataTask = NULL;

//----------------------------------------------------------------------
// DataMain
// --------
// Called when the data collection task starts
//----------------------------------------------------------------------
void DataMain(void* Unused) {
  // Set up pins
  pinMode(PIN_LOMINUS, INPUT);
  pinMode(PIN_LOPLUS, INPUT);

  // Start collecting data
  IdxADCData = 0;

  int counter = 0;
  while (true) {
    // Only write into the data buffer if a client is not reading it
    if (!client_reading_data) {
      ADCData[IdxADCData++] = analogRead(PIN_ADC);
      if (IdxADCData >= MAX_ADCDATA)
        IdxADCData = 0;
    }
    // Once every frame print the L0- and L0+ signals
    counter++;
    if (counter >= MAX_ADCDATA) {
      Serial.printf("Out = %d, L0- = %d, L0+ = %d\n", analogRead(PIN_ADC), digitalRead(PIN_LOMINUS), digitalRead(PIN_LOPLUS));
      counter = 0;
    }

    // Wait for the next data point
    delay(ADC_INTERVAL);
  }
}

//----------------------------------------------------------------------
// DataStart
// ---------
// Start the data collection task
//----------------------------------------------------------------------
bool DataStart() {
  xTaskCreate(DataMain, "DataTask", DATATASK_STACK, NULL, tskIDLE_PRIORITY + 1, &hDataTask);
  return true;
}

//----------------------------------------------------------------------
// DataRead
// --------
//----------------------------------------------------------------------
int DataRead(uint16_t data[MAX_ADCDATA]) {
  // If another thread is reading data try waiting a bit
  if (client_reading_data)
    delay(ADC_INTERVAL);

  // If after waiting the data is still in use give up
  if (client_reading_data)
    return 0;

  // Set the mutex
  client_reading_data = true;

  // Get the data
  for (int i = 0; i < MAX_ADCDATA; i++)
    data[i] = ADCData[(i + IdxADCData) % MAX_ADCDATA];

  // Free the mutex
  client_reading_data = false;

  // Return the number of data points read
  return MAX_ADCDATA;
}