// ---------------------------------------------------------------------
// Calib.cpp
// =========
// ---------------------------------------------------------------------
#include "arduino.h"
#include "driver/dac_cosine.h"

// ---------------------------------------------------------------------
// setCalibFreq
// ------------
// Set the frequency of the built in calibration cosine generator
// This creates a cosine signal varying between 0 and 1.65V
// The signal is output on pin 25
// ---------------------------------------------------------------------
bool setCalibFreq(int Freq) {
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
  cos0_cfg.atten   = DAC_COSINE_ATTEN_DB_6;

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
  Serial.printf("Calibration frequency = %d\n", Freq);
  return true;
}