//----------------------------------------------------------------------
// CYD backlight control
// =====================
//----------------------------------------------------------------------
#include <driver/ledc.h>

// TFT backlight
#define TFT_BL 21

// LEDC settings
#define LEDC_PIN            21
#define LEDC_FREQ         1000
#define XTAL_CLK_FREQ 80000000
#define SPEED_MODE LEDC_HIGH_SPEED_MODE
#define CLK_CFG    LEDC_USE_APB_CLK

//----------------------------------------------------------------------
// SetBrightness
//--------------
// The brightness is a value from 0 to 100
// These settings are for the ESP32. The settings are different for
// other ESP32 variants.
//----------------------------------------------------------------------
void SetBrightness(int Brightness) {
  // The resolution needs to be static because we set it on the first
  // call then use it in subsequent calls.
  static uint32_t resolution;

  // We only need to configure the channel once
  static bool configured = false;
  if (!configured) {
    resolution = ledc_find_suitable_duty_resolution(XTAL_CLK_FREQ, LEDC_FREQ);
    ledc_timer_config_t ledc_timer = {};
    ledc_timer.speed_mode      = LEDC_HIGH_SPEED_MODE;
    ledc_timer.duty_resolution = ledc_timer_bit_t(resolution);
    ledc_timer.timer_num       = LEDC_TIMER_0;
    ledc_timer.freq_hz         = LEDC_FREQ;
    ledc_timer.clk_cfg         = LEDC_USE_APB_CLK;
    ledc_timer_config(&ledc_timer);
    configured = true;
  }

  // Configure the channel
  ledc_channel_config_t ledc_channel = {0};
  ledc_channel.channel    = LEDC_CHANNEL_0;
  ledc_channel.duty       = (Brightness << resolution)/100;
  ledc_channel.gpio_num   = LEDC_PIN;
  ledc_channel.speed_mode = LEDC_HIGH_SPEED_MODE;
  ledc_channel.timer_sel  = LEDC_TIMER_0;
  ledc_channel_config(&ledc_channel);
}

//----------------------------------------------------------------------
// FadeUp
// ------
// Fade up the display
//----------------------------------------------------------------------
void FadeUp() {
  for (int i = 0; i <= 100; i++) {
    SetBrightness(i);
    delay(50);
  }
}

//----------------------------------------------------------------------
// FadeDown
// --------
// Fade down the display
//----------------------------------------------------------------------
void FadeDown() {
  for (int i = 100; i >= 0; i--) {
    SetBrightness(i);
    delay(50);
  }
}

//----------------------------------------------------------------------
// setup
//------
//----------------------------------------------------------------------
void setup() {
  // We need a long timeout for reading commands from the user
  Serial.setTimeout(100000);
  Serial.begin(115200);
  delay(2000);
  Serial.println("Starting Backlight");
  Serial.println("Enter a number from 0 to 100 or the commands fadeup or fadedown:");

  SetBrightness(100);
}

//----------------------------------------------------------------------
// loop
//-----
//----------------------------------------------------------------------
void loop() {
  // Read a command
  String s = Serial.readStringUntil(10);

  // Check for the fade up/down commands
  if (s == "fadeup") {
    Serial.println("Fading the display up over 5 seconds");
    FadeUp();
    Serial.println("Done!");
    return;
  }

  if (s == "fadedown") {
    Serial.println("Fading the display down over 5 seconds");
    FadeDown();
    Serial.println("Done!");
    return;
  }

  // Is this a number?
  int i = s.toInt();
  if (i < 0)
    i = 0;
  if (i > 100)
    i = 100;
  Serial.printf("Setting the brightness to %d\n", i);
  SetBrightness(i);
}