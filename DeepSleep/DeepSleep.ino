//----------------------------------------------------------------------
// This code test using the deep sleep functions.
// It schedules a wake-up then puts the ESP32 into deep sleep.
// It records the number of sleep/wake cycles in the non-volatile
// storage.
//----------------------------------------------------------------------
#include "nvs_flash.h"
#define REBOOT_PART "rhs"
#define REBOOT_KEY  "reboots"

void setup() {
  Serial.begin(115200);
  while (!Serial)
    delay(1000);
  Serial.println("Starting ...");

  // Get the number of reboots from flash
  nvs_flash_init();
  nvs_handle_t handle;
  nvs_open(REBOOT_PART, NVS_READWRITE, &handle);
  uint32_t reboots = 0;
  nvs_get_u32(handle, REBOOT_KEY, &reboots);

  // Check the cause of the reboot
  esp_sleep_wakeup_cause_t cause = esp_sleep_get_wakeup_cause();
  if (cause == ESP_SLEEP_WAKEUP_TIMER){
    Serial.println("Woken by timer");
    Serial.printf("No. reboots = %d\n", ++reboots);
  }
  else {
    Serial.println("First boot!");
    reboots = 0;
  }

  // Write the updated reboot count to flash
  nvs_set_u32(handle, REBOOT_KEY, reboots);
  nvs_commit(handle);
  nvs_close(handle);
}

void loop() {
  for (int i = 10; i > 0; i--) {
    Serial.printf("%d\n", i);
    delay(1000);
  }
  esp_sleep_enable_timer_wakeup(10000000); // time in microseconds i.e. 10 secs
  Serial.println("Calling esp_deep_sleep_start()");
  esp_deep_sleep_start();
  // This should never be called
  Serial.println("Called esp_deep_sleep_start()");
}