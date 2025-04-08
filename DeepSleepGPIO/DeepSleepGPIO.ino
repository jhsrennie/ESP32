//----------------------------------------------------------------------
// Demonstrate waking from deep sleep using a GPIO signal
//----------------------------------------------------------------------
#include <driver/rtc_io.h>

// Wake on pin 2
#define WAKEUP_GPIO GPIO_NUM_2

// Print the reason for waking
void print_wakeup_reason() {
  esp_sleep_wakeup_cause_t wakeup_reason;

  wakeup_reason = esp_sleep_get_wakeup_cause();

  switch (wakeup_reason) {
    case ESP_SLEEP_WAKEUP_EXT0:
      Serial.println("Wakeup caused by external signal using RTC_IO");
      break;
    case ESP_SLEEP_WAKEUP_EXT1:
      Serial.println("Wakeup caused by external signal using RTC_CNTL");
      break;
    case ESP_SLEEP_WAKEUP_TIMER:
      Serial.println("Wakeup caused by timer");
      break;
    case ESP_SLEEP_WAKEUP_TOUCHPAD:
      Serial.println("Wakeup caused by touchpad");
      break;
    case ESP_SLEEP_WAKEUP_ULP:
      Serial.println("Wakeup caused by ULP program");
      break;
    default:
      Serial.printf("Wakeup was not caused by deep sleep: %d\n", wakeup_reason);
      break;
  }
}

// setup
void setup() {
  Serial.begin(115200);
  delay(2000);
  Serial.println("Started!");
  print_wakeup_reason();

  // Configure wke on GIO 2
  pinMode(WAKEUP_GPIO, INPUT);
  esp_sleep_enable_ext0_wakeup(WAKEUP_GPIO, HIGH);

  // Since power will be off configure the RTCIO to pull the wake pin low
  rtc_gpio_pullup_dis(WAKEUP_GPIO);
  rtc_gpio_pulldown_en(WAKEUP_GPIO);

  // Press enter to go into deep sleep
  Serial.println("Press enter to continue");
  Serial.setTimeout(1000000); // timeout = 1000s
  String s = Serial.readStringUntil(10);
  Serial.println("Going to sleep now");
  esp_deep_sleep_start();
  Serial.println("This will never be printed");
}

// The loop function is never called
void loop() {
  Serial.println(digitalRead(WAKEUP_GPIO));
  delay(1000);
}