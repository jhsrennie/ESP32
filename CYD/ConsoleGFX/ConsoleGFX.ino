//----------------------------------------------------------------------
// Console
// -------
// Test the CYD console class
//----------------------------------------------------------------------
#include "CYDConsole.h"

CYDConsole con;

void setup() {
  Serial.begin(115200);
  delay(2000);
  Serial.println("Console");

  con.init();
}

void loop() {
  static int counter = 0;

  // Generate a random string
  if (counter % 2 == 0)
    con.printf("%d - ABCDEFGHIJKLMNOPQRST", counter);
  else
    con.printf("%d - abcdefghijklmnopqrstuvwxyz", counter);
  counter++;

  if (counter >= 9)
    delay(1000);
}