//----------------------------------------------------------------------
// cli.cpp
// =======
// Command line interface via the serial port
//----------------------------------------------------------------------
#include <Arduino.h>
#include "clock.h"

// Buffer to hold serial input
#define LEN_BUF 256
#define MAX_BUF 255 // One less than the buffer size
static char ser_buf[LEN_BUF];
static int ser_buf_pos = 0;

// Array of tokens created by parsing the input string
#define MAX_TOKENS 8
static char* token[MAX_TOKENS];
static int num_tokens = 0;

//----------------------------------------------------------------------
// ProcessCommand
//---------------
// Process the command in the tokens array
//----------------------------------------------------------------------
static void ProcessCommand() {
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
static void ProcessInput() {
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