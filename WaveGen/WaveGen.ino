#include <math.h>
#include "driver/dac_continuous.h"

#define CONST_PERIOD_2_PI     6.2832
#define LEN_WAVEFORM     400 // Length of wave array
#define DAC_AMPLITUDE 255 // Amplitude of DAC voltage

#define WAVE_FREQ_HZ    2000 // Wave frequency
#define CONVERT_FREQ_HZ (LEN_WAVEFORM * WAVE_FREQ_HZ) // Conversion frequency

// Handle to wave generator
dac_continuous_handle_t cont_handle;

// Arrays containing the waveform
uint8_t sin_wav[LEN_WAVEFORM];
uint8_t tri_wav[LEN_WAVEFORM];
uint8_t saw_wav[LEN_WAVEFORM];
uint8_t squ_wav[LEN_WAVEFORM];

// Initialise the waveform arrays
void InitWaveforms(void)
{
    uint32_t pnt_num = LEN_WAVEFORM;

    for (int i = 0; i < pnt_num; i ++) {
        sin_wav[i] = (uint8_t)((sin( i * CONST_PERIOD_2_PI / pnt_num) + 1) * (double)(DAC_AMPLITUDE) / 2 + 0.5);
        tri_wav[i] = (i > (pnt_num / 2)) ? (2 * DAC_AMPLITUDE * (pnt_num - i) / pnt_num) : (2 * DAC_AMPLITUDE * i / pnt_num);
        saw_wav[i] = (i == pnt_num) ? 0 : (i * DAC_AMPLITUDE / pnt_num);
        squ_wav[i] = (i < (pnt_num / 2)) ? DAC_AMPLITUDE : 0;
    }
}

// Initialise the DMA wave generation
bool InitWaveGen(void)
{
  dac_continuous_config_t cont_cfg = {
    .chan_mask = DAC_CHANNEL_MASK_ALL,
    .desc_num = 8,
    .buf_size = 2048,
    .freq_hz = CONVERT_FREQ_HZ,
    .offset = 0,
    .clk_src = DAC_DIGI_CLK_SRC_DEFAULT, // If the frequency is out of range, try 'DAC_DIGI_CLK_SRC_APLL'
    .chan_mode = DAC_CHANNEL_MODE_SIMUL,
  };
  // Allocate new DAC channels in continuous mode
  esp_err_t e = dac_continuous_new_channels(&cont_cfg, &cont_handle);
  if (e != ESP_OK) {
    Serial.printf("dac_continuous_new_channels returned %d\n", e);
    return false;
  }
  /* Enable the channels in the group */
  e = dac_continuous_enable(cont_handle);
  if (e != ESP_OK) {
    Serial.printf("dac_continuous_enable returned %d\n", e);
    return false;
  }

  // Ready to go
  // Call dac_continuous_write_cyclically() to start the conversion
  return true;
}

void setup() {
  Serial.begin(115200);
  delay(2000);
  Serial.println("WaveGen starting");

  // Initialise the waveform generation
  Serial.printf("Frequency = %d Hz\n", WAVE_FREQ_HZ);
  InitWaveforms();
  if (!InitWaveGen()) {
    Serial.println("Initialisation failed, suspending task ...");
    vTaskSuspend(NULL);
  }
}

void loop() {
  esp_err_t e;

  // We will loop over all four waveforms displaying each for 10s
  for (int i = 0; i < 4; i++) {
    switch (i) {
      case 0:
        Serial.println("Sine wave");
        e = dac_continuous_write_cyclically(cont_handle, sin_wav, LEN_WAVEFORM, NULL);
        break;

      case 1:
        Serial.println("Triangular wave");
        e = dac_continuous_write_cyclically(cont_handle, tri_wav, LEN_WAVEFORM, NULL);
        break;

      case 2:
        Serial.println("Sawtooth wave");
        e = dac_continuous_write_cyclically(cont_handle, saw_wav, LEN_WAVEFORM, NULL);
        break;

      case 3:
        Serial.println("Square wave");
        e = dac_continuous_write_cyclically(cont_handle, squ_wav, LEN_WAVEFORM, NULL);
        break;
    }
    if (e != ESP_OK) {
      Serial.printf("dac_continuous_write_cyclically returned %d\n", e);
      return;
    }

    // Show the waveform for 5s
    delay(5000);
  }

  // To disable channels use
  // dac_continuous_del_channels(dac_continuous_handle_t handle)
  // To stop continuous dma use
  // dac_continuous_disable(dac_continuous_handle_t handle)
}
