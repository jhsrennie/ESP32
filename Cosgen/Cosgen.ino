// ---------------------------------------------------------------------
// This code shows how to use the cosine generator built into the
// ESP32. It reads a frequency typed by the user into the serial port
// and outputs a cosine wave of that frequency on pin 25.
// ---------------------------------------------------------------------
#include "driver/dac_cosine.h"

// Channel handle
dac_cosine_handle_t chan0_handle = nullptr;

// Start generating a cosine wave
bool setFreq(int Freq) {
  // Check the frequency
  if (Freq < 130) {
    Serial.printf("Requested frequency %d is lower than the limit of 130\n", Freq);
    return false;
  }

  // If a channel is currently active close is
  if (chan0_handle) {
    dac_cosine_stop(chan0_handle);
    dac_cosine_del_channel(chan0_handle);
    chan0_handle = nullptr;
  }

  // Set the new frequency
  dac_cosine_config_t cos0_cfg;
  memset(&cos0_cfg, 0, sizeof(cos0_cfg));
  cos0_cfg.chan_id = DAC_CHAN_0; // DAC channel zero is pin 25
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

void setup() {
  Serial.begin(115200);
  while (!Serial)
    delay(1000);
  // Use a serial input timeout of 1000s i.e. infinite
  Serial.setTimeout(1000000);

  Serial.println("Cosine generator starting ...");
}

void loop() {
  // Read the frequency
  Serial.println("Enter the frequency:");
  long freq = Serial.parseInt();
  // Set the frequency
  setFreq(freq);
}
