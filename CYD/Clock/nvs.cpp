//----------------------------------------------------------------------
// nvs.cpp
// =======
// Functions to read and write from the non-volatile storage
//----------------------------------------------------------------------
#include <Arduino.h>
#include <nvs_flash.h>
#include "clock.h"

// The maximum wi-fi SSID length is 31 bytes
#define MAX_LEN_SSID 31
// The maximum wi-fi password length is 63 bytes
#define MAX_LEN_PWD 63

// The NVS is used to store clock settings
#define CLOCK_NVS_PART "ClockNVS"
#define KEY_SSID "ssid"
#define KEY_PWD  "pass"
nvs_handle_t h_nvs = NULL;

// WiFi defaults
#define DEFAULT_SSID "SSID"
#define DEFAULT_PWD  "password"

//----------------------------------------------------------------------
// InitClockNVS
// ------------
// Initialise the clock storage
//----------------------------------------------------------------------
bool InitClockNVS() {
  // If the flash has been initialised we can just return
  if (h_nvs)
    return true;

  // Otherwise initialise the flash
  esp_err_t e = nvs_flash_init();
  if (e != ESP_OK) {
    Serial.printf("InitClockNVS error: %s\n", esp_err_to_name(e));
    return false;
  }
  e = nvs_open(CLOCK_NVS_PART, NVS_READWRITE, &h_nvs);
  if (e != ESP_OK) {
    Serial.printf("InitClockNVS error: %s\n", esp_err_to_name(e));
    return false;
  }

  // Return indicating success
  return true;
}

//----------------------------------------------------------------------
// SetSSID
// -------
// Write the SSID into NVS
//----------------------------------------------------------------------
bool SetSSID(const char* SSID) {
  // Check the SSID length
  if (strlen(SSID) > MAX_LEN_SSID) {
    Serial.printf("SetSSID error: SSID is longer than the maximum of %d characters\n", MAX_LEN_SSID);
    return false;
  }

  // Check the flash is initialised
  if (!h_nvs)
    if (!InitClockNVS())
      return false;

  // Set the SSID
  esp_err_t e = nvs_set_str(h_nvs, KEY_SSID, SSID);
  if (e == ESP_OK) {
    Serial.printf("SSID set to %s\n", SSID);
    return true;
  }
  Serial.printf("SetSSID error: %s\n", esp_err_to_name(e));
  return false;
}

//----------------------------------------------------------------------
// GetSSID
// -------
// Read the SSID from NVS
//----------------------------------------------------------------------
const char* GetSSID() {
  // Use a static buffer so we can return a const char*
  static size_t len_ssid;
  static char ssid[MAX_LEN_SSID+1];

  // Check the flash is initialised
  if (!h_nvs)
    if (!InitClockNVS())
      return DEFAULT_SSID;

  // Get the SSID
  len_ssid = MAX_LEN_SSID+1;
  esp_err_t e = nvs_get_str(h_nvs, KEY_SSID, ssid, &len_ssid);
  if (e == ESP_OK) {
    Serial.printf("SSID is %s\n", ssid);
    return ssid;
  }
  Serial.printf("GetSSID error: %s\n", esp_err_to_name(e));
  return DEFAULT_SSID;
}

//----------------------------------------------------------------------
// SetPassword
// -----------
// Write the password into NVS
//----------------------------------------------------------------------
bool SetPassword(const char* Password) {
  // Check the Password length
  if (strlen(Password) > MAX_LEN_PWD) {
    Serial.printf("SetPassword error: Password is longer than the maximum of %d characters\n", MAX_LEN_PWD);
    return false;
  }

  // Check the flash is initialised
  if (!h_nvs)
    if (!InitClockNVS())
      return false;

  // Set the Password
  esp_err_t e = nvs_set_str(h_nvs, KEY_PWD, Password);
  if (e == ESP_OK) {
    Serial.printf("Password set to %s\n", Password);
    return true;
  }
  Serial.printf("GetPassword error: %s\n", esp_err_to_name(e));
  return false;
}

//----------------------------------------------------------------------
// GetPassword
// -------
// Read the Password from NVS
//----------------------------------------------------------------------
const char* GetPassword() {
  // Use a static buffer so we can return a const char*
  static size_t len_password;
  static char password[MAX_LEN_PWD+1];

  // Check the flash is initialised
  if (!h_nvs)
    if (!InitClockNVS())
      return DEFAULT_PWD;

  // Get the Password
  len_password = MAX_LEN_PWD+1;
  esp_err_t e = nvs_get_str(h_nvs, KEY_PWD, password, &len_password);
  if (e == ESP_OK) {
    Serial.printf("Password is %s\n", password);
    return password;
  }
  Serial.printf("GetPassword error: %s\n", esp_err_to_name(e));
  return DEFAULT_PWD;
}