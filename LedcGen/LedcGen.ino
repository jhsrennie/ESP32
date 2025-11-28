//----------------------------------------------------------------------
// Example of using the ledc generator
//----------------------------------------------------------------------
#include "driver/ledc.h"

// Output the square wave on pin 25
#define LEDC_PIN 12

// Just to be annoying the original ESP32 works slightly differently
// from the later ones.
#if CONFIG_IDF_TARGET_ESP32
#define XTAL_CLK_FREQ 80000000
#define SPEED_MODE LEDC_HIGH_SPEED_MODE
#define CLK_CFG    LEDC_USE_APB_CLK
#else
#define SPEED_MODE LEDC_LOW_SPEED_MODE
#define CLK_CFG    LEDC_USE_XTAL_CLK
#endif

// Save the frequency and duty cycle as globals
uint32_t Frequency = 1000, DutyCycle = 50;

// Start generating a square wave using ledc
bool SetFreq(uint32_t Freq, uint32_t Duty) {
  Serial.printf("SetFreq(%d, %d)\n", Freq, Duty);

  // We need the number of internal clock ticks per cycle of the LEDC output
  int ticks_per_cycle = XTAL_CLK_FREQ/Freq;

  // The duty_resolution setting in ledc_timer_config_t is the number of
  // bits needed for the number of clock ticks per cycle. Note that it
  // cannot exceed 20 bits.
  int i = ticks_per_cycle >> 2;
  int duty_resolution = 1;
  while (i > 0 && duty_resolution < 20) {
    i = i >> 1;
    duty_resolution++;
  }

  // From the duty_resolution we get the "length" of the cycle as 2^duty_resolution
  int len_cycle = 1 << duty_resolution;
  // Then the duty_cycle setting in ledc_channel_config_t is the part of
  // this length that the signal should be high.
  // The Duty argument is a percentage, e.g. 50 means 50/100, so multiply
  // the cycle length by this percentage to get the duty_cycle value.
  uint32_t duty_cycle = (len_cycle*Duty)/100;

  // Display the settings for info
  Serial.printf("Ticks per cycle = %d\n", ticks_per_cycle);
  Serial.printf("Bits per cycle  = %d\n", duty_resolution);
  Serial.printf("Duty cycle      = %d\n", duty_cycle);

  // Configure the Ledc timer
  ledc_timer_config_t ledc_timer = {};
  ledc_timer.speed_mode      = SPEED_MODE;
  ledc_timer.duty_resolution = ledc_timer_bit_t(duty_resolution);
  ledc_timer.timer_num       = LEDC_TIMER_0;
  ledc_timer.freq_hz         = Freq;
  ledc_timer.clk_cfg         = CLK_CFG;
  ledc_timer_config(&ledc_timer);

  // Configure the channel
  ledc_channel_config_t ledc_channel = {0};
  ledc_channel.channel    = LEDC_CHANNEL_0;
  ledc_channel.duty       = duty_cycle;
  ledc_channel.gpio_num   = LEDC_PIN;
  ledc_channel.speed_mode = SPEED_MODE;
  ledc_channel.timer_sel  = LEDC_TIMER_0;
  ledc_channel_config(&ledc_channel);

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

  Serial.println("Ledc generator starting ...");
  SetFreq(1000, 50);
}

void loop() {
  int freq, duty;

  // Read the frequency
  Serial.println("Enter the frequency:");
  int i = Serial.parseInt();
  // If it wasn't an int report an error
  if (i == 0) {
    Serial.println("Invalid integer");
    return;
  }

  // Horrible hack, positive numbers set the frequency and negative
  // numbers set the duty cycle
  if (i > 0) {
    if (SetFreq(i, DutyCycle))
      Frequency = i;
  }

  else if (i < 0) {
    i = -i;
    if (SetFreq(Frequency, i))
      DutyCycle = i;
  }
}