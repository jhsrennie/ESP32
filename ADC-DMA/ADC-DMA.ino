// ---------------------------------------------------------------------
// Test of continuous ADC.
// This uses the cosine generator to produce a cosine wave on pin 25
// then it uses continuous ADC to read the signal on pin 36.
// This was written to test how continuous ADC is used.
// ---------------------------------------------------------------------
#include "esp_adc/adc_continuous.h"
#include "driver/dac_cosine.h"

#define ADC_BUFFER_SIZE 1024
#define ADC_FRAME_SIZE  256
#define ADC_NUM_SAMPLES 1024

// ADC handle
adc_continuous_handle_t adc_handle = NULL;

// ---------------------------------------------------------------------
// Start generating a cosine wave
// This uses channel 1 so it outputs on pin 25
// ---------------------------------------------------------------------
bool setFreq(int Freq) {
  dac_cosine_handle_t chan0_handle = nullptr;

  // Check the frequency
  if (Freq < 130) {
    Serial.printf("Requested frequency %d is lower than the limit of 130\n", Freq);
    return false;
  }

  // Set the new frequency
  dac_cosine_config_t cos0_cfg;
  memset(&cos0_cfg, 0, sizeof(cos0_cfg));
  cos0_cfg.chan_id = DAC_CHAN_0;
  cos0_cfg.freq_hz = Freq;
  cos0_cfg.clk_src = DAC_COSINE_CLK_SRC_DEFAULT;
  cos0_cfg.offset  = 0;
  cos0_cfg.phase   = DAC_COSINE_PHASE_0;
  cos0_cfg.atten   = DAC_COSINE_ATTEN_DEFAULT;

  esp_err_t e = dac_cosine_new_channel(&cos0_cfg, &chan0_handle);
  if (e != ESP_OK) {
    Serial.printf("dac_cosine_new_channel returned %d\n", e);
    return false;
  }

  e = dac_cosine_start(chan0_handle);
  if (e != ESP_OK) {
    Serial.printf("dac_cosine_start returned %d\n", e);
    return false;
  }

  // Successfully initialised
  Serial.printf("Frequency = %d\n", Freq);
  return true;
}

// ---------------------------------------------------------------------
// Initialise the ADC
// We use ADC 1 channel 0 so it's pin 36
// ---------------------------------------------------------------------
bool init_adc(adc_channel_t channel, adc_continuous_handle_t* ret_handle) {
  adc_continuous_handle_t handle;

  // Get a new handle
  adc_continuous_handle_cfg_t adc_config = {
    .max_store_buf_size = ADC_BUFFER_SIZE,
    .conv_frame_size    = ADC_FRAME_SIZE,
  };
  Serial.println("Calling adc_continuous_new_handle ...");
  esp_err_t e = adc_continuous_new_handle(&adc_config, &handle);
  if (e != ESP_OK) {
    Serial.printf("adc_continuous_new_handle returned %d\n", e);
    return false;
  }

  // We need an array with a single pattern
  adc_digi_pattern_config_t adc_pattern[1] = {0};
  adc_pattern[0].atten = ADC_ATTEN_DB_11;
  adc_pattern[0].channel = ADC_CHANNEL_0; // channel 0
  adc_pattern[0].unit = ADC_UNIT_1;       // ADC 1, so pin 36
  adc_pattern[0].bit_width = SOC_ADC_DIGI_MAX_BITWIDTH;

  // Configure the A2D
  adc_continuous_config_t adc_cfg = {
    .pattern_num    = 1,
    .adc_pattern    = adc_pattern,
    .sample_freq_hz = 20*1000,
    .conv_mode      = ADC_CONV_SINGLE_UNIT_1,
    .format         = ADC_DIGI_OUTPUT_FORMAT_TYPE1,
  };

  Serial.println("Calling adc_continuous_config ...");
  e = adc_continuous_config(handle, &adc_cfg);
  if (e != ESP_OK) {
    Serial.printf("adc_continuous_config returned %d\n", e);
    return false;
  }

  // Set the ADC handle
  *ret_handle = handle;

  // All done
  Serial.println("ADC initialisation completed successfully");
  return true;
}

// ---------------------------------------------------------------------
// Get one frame and write the data to the serial monitor
// ---------------------------------------------------------------------
bool read_frame(void) {
  // Start the conversion
  Serial.println("Calling adc_continuous_start ...");
  esp_err_t e = adc_continuous_start(adc_handle);
  if (e != ESP_OK) {
    Serial.printf("adc_continuous_start returned %d\n", e);
    return false;
  }

  // Read a single frame
  uint32_t bytes_to_read, bytes_read, total_read;
  uint8_t read_buf[ADC_NUM_SAMPLES*sizeof(adc_digi_output_data_t)] = {0};

  bytes_to_read = ADC_NUM_SAMPLES*sizeof(adc_digi_output_data_t);
  total_read = bytes_read = 0;
  while (total_read < bytes_to_read) {
    Serial.printf("Calling adc_continuous_read(%d)", bytes_to_read);
    e = adc_continuous_read(adc_handle, read_buf + total_read, bytes_to_read - total_read, &bytes_read, ADC_MAX_DELAY);
    if (e != ESP_OK) {
      Serial.printf("adc_continuous_read returned %d\n", e);
      return false;
    }
    Serial.printf("No. bytes read = %d\n", bytes_read);
    total_read += bytes_read;
  }

  // Stop the ADC
  adc_continuous_stop(adc_handle);

  // Print the data
  adc_digi_output_data_t* data = (adc_digi_output_data_t*) read_buf;
  for (int i = 0; i < ADC_NUM_SAMPLES; i++)
    Serial.printf("%d\n", data[i].type1.data);

  // Return indicating success
  return true;
}

// ---------------------------------------------------------------------
// setup
// ---------------------------------------------------------------------
void setup() {
  Serial.begin(115200);
  while (!Serial)
    delay(1000);
  Serial.println("Continuous A2D starting ...");

  // Start the frequency generator
  setFreq(200);

  // Initialise the ADC
  init_adc(ADC_CHANNEL_0, &adc_handle);
}

// ---------------------------------------------------------------------
// loop
// ---------------------------------------------------------------------
void loop() {
  delay(10000);
  read_frame();
  vTaskSuspend(NULL);
}