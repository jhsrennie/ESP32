//----------------------------------------------------------------------
// CYD Clock program
// =================
//----------------------------------------------------------------------
#include "clock.h"

//----------------------------------------------------------------------
// setup
//------
//----------------------------------------------------------------------
void setup() {
  Serial.begin(115200);
  delay(2000);
  Serial.println("Starting Clock");

  // Initialise the display
  DisplayStatus("Initialising the display ...");
  InitDisplay();

  // Start the task to get the time
  DisplayStatus("Initialising the time ...");
  TaskHandle_t h;
  xTaskCreate(UpdateTime, "UpdateTime", 4000, NULL, 2, &h);

  // Initialise the sensors
  Serial.println("Initialising sensors ...");
  InitSensors();

  // All done
  Serial.println("Clock started");
}

//----------------------------------------------------------------------
// loop
//-----
//----------------------------------------------------------------------
void loop() {
  static int brightness = 100;

  // The time updates every second
  static int timecount = 0;
  if (timecount == 0)
    DisplayTime();
  timecount = (timecount + 1) % 10;

  // If the touchscreen is touched ramp up the brightness
  if(IsTouched())
    brightness = 100;

  // Gradually dim the screen
  if (brightness >= 5)
    SetBrightness(brightness);
  if (brightness > 0)
    brightness -= 1;

  // The temperature updates every 60 seconds
  static int tempcount = 0;
  if (tempcount == 0) {
    GetTemp();
    DisplayTemp();
  }
  tempcount = (tempcount + 1) % 600;

  // Check if there's any serial input to process
  if (Serial.available() > 0)
    ProcessSerial();

  // All done for now
  delay(100);
}