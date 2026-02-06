//----------------------------------------------------------------------
// Console
// -------
//----------------------------------------------------------------------
#include <CYDConsole.h>

CYDConsole con;

void setup() {
  Serial.begin(115200);
  delay(2000);
  Serial.println("Console");

  con.init();
}

void loop() {
  static int counter = 0;

  con.printf("time = %d", counter++);

  if (counter >= 14)
    delay(1000);
}