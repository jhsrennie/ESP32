//----------------------------------------------------------------------
// Demonstrate waking the C3 from deep sleep using GPIO 2
//----------------------------------------------------------------------
#include "esp_sleep.h"

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
    case ESP_SLEEP_WAKEUP_GPIO:
      Serial.println("Wakeup caused by GPIO");
      break;
    default:
      Serial.printf("Wakeup was not caused by deep sleep: %d\n", wakeup_reason);
      break;
  }
}

// setup
void setup() {
  esp_err_t e;

  Serial.begin(115200);
  delay(2000);
  Serial.println("Started!");
  print_wakeup_reason();

  // Configure a wake after 30 secs for debugging
  e = esp_sleep_enable_timer_wakeup(30000000);
  if (e != ESP_OK) {
    Serial.printf("Error: esp_sleep_enable_timer_wakeup() returned %d\n", e);
    return;
  }

  // Configure wake on GPIO
  pinMode(WAKEUP_GPIO, INPUT);
  e = esp_deep_sleep_enable_gpio_wakeup(BIT(WAKEUP_GPIO), ESP_GPIO_WAKEUP_GPIO_HIGH);
  if (e != ESP_OK) {
    Serial.printf("Error: esp_deep_sleep_enable_gpio_wakeup() returned %d\n", e);
    return;
  }
  
  // Press enter to go into deep sleep
  Serial.println("Press enter to continue");
  Serial.setTimeout(1000000); // timeout = 1000s
  String s = Serial.readStringUntil(10);

  // Enter deep sleep
  Serial.println("Going to sleep now");
  esp_deep_sleep_start();
  Serial.println("This will never be printed");
}

// The loop function is never called
void loop() {
  Serial.println(digitalRead(WAKEUP_GPIO));
  delay(1000);
}