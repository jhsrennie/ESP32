//----------------------------------------------------------------------
// Example of using the ledc generator
//----------------------------------------------------------------------
#include "driver/ledc.h"

// Output the square wave on pin 25
#define LEDC_PIN 25

// The APB clock is 80MHz
#define APB_FREQ 80000000

// Save the frequency and duty cycle as globals
uint32_t Frequency = 1000, DutyCycle = 50;

// Start generating a square wave using ledc
bool SetFreq(uint32_t Freq, uint32_t Duty) {
  // The duty cycle resolution is the number of bits needed for the
  // clock divider, but it cannot exceed 20 bits.
  uint32_t clk_divider = APB_FREQ/Freq;
  Serial.printf("Clock divider = %d\n", clk_divider);
  uint32_t duty_res_bits = (uint32_t) (log(clk_divider)/log(2));

  // Sanity check the duty cycle resolution
  if (duty_res_bits > 20)
    duty_res_bits = 20;
  if (duty_res_bits < 1)
    duty_res_bits = 1;
  Serial.printf("Clock resolution = %d\n", duty_res_bits);

  // And the resolution is 2^resbits
  uint32_t duty_resolution = 2 << (duty_res_bits - 1);

  // Then the duty cycle is expressed as a fraction of the resolution
  // The Duty argument is a percentage e.g. 50 means 50/100
  uint32_t duty = (Duty*duty_resolution)/100;
  Serial.printf("Duty cycle = %d\n", duty);

  // Configure the Ledc timer
  ledc_timer_config_t ledc_timer = {};
  ledc_timer.speed_mode      = LEDC_HIGH_SPEED_MODE;
  ledc_timer.duty_resolution = ledc_timer_bit_t(duty_res_bits);
  ledc_timer.timer_num       = LEDC_TIMER_0;
  ledc_timer.freq_hz         = Freq;
  ledc_timer.clk_cfg         = LEDC_USE_APB_CLK;
  ledc_timer_config(&ledc_timer);

  // Configure the channel
  ledc_channel_config_t ledc_channel = {};
  ledc_channel.channel    = LEDC_CHANNEL_0;
  ledc_channel.duty       = duty;
  ledc_channel.gpio_num   = LEDC_PIN;
  ledc_channel.intr_type  = LEDC_INTR_DISABLE;
  ledc_channel.speed_mode = LEDC_HIGH_SPEED_MODE;
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