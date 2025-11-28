// ---------------------------------------------------------------------
// ADC.cpp
// =======
// Continuous ADC code
// The input is ADC 1 channel 0 so pin 36
// ---------------------------------------------------------------------
#include "arduino.h"
#include "esp_adc/adc_continuous.h"

#define ADC_BUFFER_SIZE 2000
#define ADC_FRAME_SIZE  500

// ADC handle
static adc_continuous_handle_t adc_handle = NULL;

// ---------------------------------------------------------------------
// init_adc
// --------
// Do all the initialisation required before starting the conversion
// ---------------------------------------------------------------------
bool init_adc(adc_channel_t Channel, int Freq) {
  // remember the last frequency used since we only need to reinitialise
  // if the frequency has changed
  static int last_freq = 0;
  if (last_freq == Freq && adc_handle)
    return true;
  last_freq = Freq;

  // If we have a handle open we need to close it
  if (adc_handle) {
    adc_continuous_deinit(adc_handle);
    adc_handle = NULL;
  }

  // Get a new handle
  adc_continuous_handle_cfg_t adc_config = {
    .max_store_buf_size = ADC_BUFFER_SIZE,
    .conv_frame_size    = ADC_FRAME_SIZE,
  };
  esp_err_t e = adc_continuous_new_handle(&adc_config, &adc_handle);
  if (e != ESP_OK) {
    Serial.printf("adc_continuous_new_handle returned %d\n", e);
    return false;
  }

  // We need an array with a single pattern
  adc_digi_pattern_config_t adc_pattern[1] = {0};
  adc_pattern[0].atten = ADC_ATTEN_DB_11;
  adc_pattern[0].channel = ADC_CHANNEL_0; // channel & 0x7;
  adc_pattern[0].unit = ADC_UNIT_1;
  adc_pattern[0].bit_width = SOC_ADC_DIGI_MAX_BITWIDTH;

  // Configure the A2D
  adc_continuous_config_t adc_cfg = {
    .pattern_num    = 1,
    .adc_pattern    = adc_pattern,
    .sample_freq_hz = Freq*1000,
    .conv_mode      = ADC_CONV_SINGLE_UNIT_1,
    .format         = ADC_DIGI_OUTPUT_FORMAT_TYPE1,
  };

  e = adc_continuous_config(adc_handle, &adc_cfg);
  if (e != ESP_OK) {
    Serial.printf("adc_continuous_config returned %d\n", e);
    return false;
  }

  // Return indicating success
  return true;
}

// ---------------------------------------------------------------------
// getFrame
// --------
// Read data from the ADC
// ---------------------------------------------------------------------
bool getFrame(uint16_t* Data, uint32_t LenData) {
  uint32_t total_bytes_to_read, total_read;
  uint32_t bytes_to_read, bytes_read;
  uint8_t* read_buf = (uint8_t*) Data;
  esp_err_t e;

  // Work out how much data we need to read
  total_bytes_to_read = LenData*sizeof(adc_digi_output_data_t);
  total_read = 0;

  // Flush the pool. If there's an error just carry on.
  // For some reason this is missing from the current header file
  // e = adc_continuous_flush_pool(adc_handle);
  // if (e != ESP_OK) {
  //   Serial.printf("adc_continuous_flush_pool returned %d\n", e);
  // }

  // Start the conversion
  e = adc_continuous_start(adc_handle);
  if (e != ESP_OK) {
    Serial.printf("adc_continuous_start returned %d\n", e);
    return false;
  }

  // Read a single frame
  while (total_read < total_bytes_to_read) {
    bytes_to_read = total_bytes_to_read - total_read;
    if (bytes_to_read > ADC_FRAME_SIZE)
      bytes_to_read = ADC_FRAME_SIZE;

    e = adc_continuous_read(adc_handle, read_buf + total_read, bytes_to_read, &bytes_read, ADC_MAX_DELAY);
    if (e != ESP_OK) {
      Serial.printf("adc_continuous_read returned %d\n", e);
      adc_continuous_stop(adc_handle);
      return false;
    }
    total_read += bytes_read;
  }

  // Stop the ADC
  adc_continuous_stop(adc_handle);

  // Return indicating success
  return true;
}

// ---------------------------------------------------------------------
// initADC
// -------
// Initialise the continuous ADC
// ---------------------------------------------------------------------
bool initADC(int Freq) {
  // Initialise the ADC
  if (!init_adc(ADC_CHANNEL_0, Freq)) {
    return false;
  }

  // Return indicating success
  return true;
}