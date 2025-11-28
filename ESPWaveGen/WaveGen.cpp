// ---------------------------------------------------------------------
// WaveGen.cpp
// ===========
// Waveform generator
// Valid frequencies are 19.6 KHz to about 2.5 MHz
// ---------------------------------------------------------------------

#include <math.h>
#include "Arduino.h"
#include "driver/dac_continuous.h"

#define CONST_2PI     6.2832
#define LEN_WAVEFORM  500 // Length of wave array
#define DAC_AMPLITUDE 255 // Amplitude of DAC voltage

// Handle to wave generator
static dac_continuous_handle_t cont_handle = NULL;

// Arrays containing the waveform
static uint8_t sin_wav[LEN_WAVEFORM];
static uint8_t tri_wav[LEN_WAVEFORM];
static uint8_t saw_wav[LEN_WAVEFORM];
static uint8_t squ_wav[LEN_WAVEFORM];

// The waveform length needs to be reduced for high frequencies
// This holds the current waveform length
static int lenWaveform = LEN_WAVEFORM;

// Initialise the waveform arrays
static void InitWaveforms(int Length)
{
  Serial.printf("Initialising waveforms with length %d\n", Length);
  for (int i = 0; i < Length; i++) {
    sin_wav[i] = (uint8_t)((sin(i*CONST_2PI/Length) + 1)*(double)(DAC_AMPLITUDE)/2 + 0.5);
    tri_wav[i] = (i > (Length/2)) ? (2*DAC_AMPLITUDE*(Length - i)/Length) : (2*DAC_AMPLITUDE*i/Length);
    saw_wav[i] = (i == Length) ? 0 : (i*DAC_AMPLITUDE/Length);
    squ_wav[i] = (i < (Length/2)) ? DAC_AMPLITUDE : 0;
  }
  lenWaveform = Length;
}

// Initialise the DMA wave generation
bool InitWaveGen(int Type, int Frequency)
{
  // Initialise the waveform arrays
  // These buffer sizes all the ftequency range 40Hz to 200KHz
  int length = LEN_WAVEFORM;
  if (Frequency >   4000) length = 200;
  if (Frequency >  10000) length = 100;
  if (Frequency >  20000) length =  50;
  if (Frequency >  40000) length =  20;
  if (Frequency > 100000) length =  10;
  InitWaveforms(length);

  // If the wave generator has been enabled we need to disable it
  if (cont_handle) {
    dac_continuous_disable(cont_handle);
    dac_continuous_del_channels(cont_handle);
    cont_handle = NULL;
  }

  // Allocate new DAC channels in continuous mode
  dac_continuous_config_t cont_cfg = {
    .chan_mask = DAC_CHANNEL_MASK_ALL,
    .desc_num = 8,
    .buf_size = 2048,
    .freq_hz = Frequency*length,
    .offset = 0,
    .clk_src = DAC_DIGI_CLK_SRC_DEFAULT,
    .chan_mode = DAC_CHANNEL_MODE_SIMUL,
  };
  esp_err_t e = dac_continuous_new_channels(&cont_cfg, &cont_handle);
  if (e != ESP_OK) {
    Serial.printf("dac_continuous_new_channels returned %d\n", e);
    return false;
  }

  // Enable the channels in the group
  e = dac_continuous_enable(cont_handle);
  if (e != ESP_OK) {
    Serial.printf("dac_continuous_enable returned %d\n", e);
    dac_continuous_del_channels(cont_handle);
    cont_handle = NULL;
    return false;
  }

  // Start the reqested type
  switch (Type) {
    case 1:
      Serial.printf("Triangular wave %d Hz\n", Frequency);
      e = dac_continuous_write_cyclically(cont_handle, tri_wav, length, NULL);
      break;

    case 2:
      Serial.printf("Sawtooth wave %d Hz\n", Frequency);
      e = dac_continuous_write_cyclically(cont_handle, saw_wav, length, NULL);
      break;

    case 3:
      Serial.printf("Square wave %d Hz\n", Frequency);
      e = dac_continuous_write_cyclically(cont_handle, squ_wav, length, NULL);
      break;

    default:
      Serial.printf("Sine wave %d Hz\n", Frequency);
      e = dac_continuous_write_cyclically(cont_handle, sin_wav, length, NULL);
      break;
  }
  if (e != ESP_OK) {
    Serial.printf("dac_continuous_write_cyclically returned %d\n", e);
    return false;
  }

  // Return indicating success
  return true;
}