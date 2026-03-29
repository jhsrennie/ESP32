//----------------------------------------------------------------------
// utils.cpp
// =========
// Various functions used by the clock
//----------------------------------------------------------------------
#include <Arduino.h>
#include <nvs_flash.h>
#include "clock.h"

// Buffer to hold serial input
#define LEN_BUF 256
#define MAX_BUF 255 // One less than the buffer size
char ser_buf[LEN_BUF];
int ser_buf_pos = 0;

// Array of tokens created by parsing the input string
#define MAX_TOKENS 8
char* token[MAX_TOKENS];
int num_tokens = 0;

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

//----------------------------------------------------------------------
// ProcessCommand
//---------------
// Process the command in the tokens array
//----------------------------------------------------------------------
void ProcessCommand() {
  // Sanity check
  if (num_tokens == 0)
    return;

  // Reboot
  if (strcasecmp("reboot", token[0]) == 0 || strcasecmp("restart", token[0]) == 0) {
    Serial.println("Rebooting ...");
    ESP.restart();
  }

  // Set the SSID
  else if (strcasecmp("ssid", token[0]) == 0) {
    if (num_tokens != 2) {
      Serial.println("Syntax: ssid <yourssid>");
    }
    SetSSID(token[1]);
  }

  // Set the wi-fi password
  else if (strcasecmp("password", token[0]) == 0) {
    if (num_tokens != 2) {
      Serial.println("Syntax: password <yourpassword>");
    }
    SetPassword(token[1]);
  }

  // Unknown command
  else {
    Serial.printf("Unknown command: %s", token[0]);
    for (int i = 1; i < num_tokens; i++)
      Serial.printf(" %s", token[i]);
    Serial.println("");
  }

  // Reset the tokens
  num_tokens = 0;  
}

//----------------------------------------------------------------------
// ProcessInput
//-------------
// Called when a \n is received to process the input buffer into an
// array of tokens
//----------------------------------------------------------------------
void ProcessInput() {
  // Terminate the command
  ser_buf[ser_buf_pos] = '\0';

  // Parse the command into tokens
  num_tokens = 0;
  for (char* p = ser_buf; *p != '\0'; p++) {
    // Find the next non-space character
    while (*p == ' ')
      p++;
    // Check for end of string
    if (*p == '\0')
      break;

    // Set the token
    token[num_tokens++] = p;

    // Find the end of the token
    while (*p != ' ' && *p != '\0')
      p++;
    // Terminate the token
    if (*p != '\0')
      *p = '\0';
  
    // Check the number of tokens
    if (num_tokens >= MAX_TOKENS)
      break;
  }

  // Process the command
  ProcessCommand();

  // And clear the input buffer
  ser_buf_pos = 0;
}

//----------------------------------------------------------------------
// ProcessSerial
//--------------
// This is called from the loop() function whenever serial input is
// available.
//----------------------------------------------------------------------
void ProcessSerial() {
  // Get the pending serial input
  String s = Serial.readString();

  // Copy characters to the input buffer
  for (const char* p = s.c_str(); *p; p++) {
    // \n marks the end of a command
    if (*p == '\n') {
      ProcessInput();
      continue;
    }

    // Copy the character into the input buffer
    if (ser_buf_pos < MAX_BUF)
      ser_buf[ser_buf_pos++] = *p;
  }
}