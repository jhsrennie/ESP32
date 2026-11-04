//**********************************************************************
// SSD1306
// =======
// Example sketch to show how to use the SSD1306 display on an ESP32
// using the Arduino GFX library.
//**********************************************************************
#include <Wire.h>
#include <Adafruit_GFX.h>
#include <Adafruit_SSD1306.h>

// The I2C address of the SSD1306 is 0x3C
#define I2C_ADDRESS 0x3C

// Screen size
#define SCREEN_WIDTH 128
#define SCREEN_HEIGHT 64

// GPIO pins
#define PIN_RESET -1 // We don't use the reset so set it to -1
#define PIN_SDA   21
#define PIN_SCL   22

// Create the display object
Adafruit_SSD1306 ssd1306(SCREEN_WIDTH, SCREEN_HEIGHT, &Wire, PIN_RESET);

//----------------------------------------------------------------------
// setup
// -----
//----------------------------------------------------------------------
void setup() {
  // Initialise the serial monitor and wait for it to complete
  Serial.begin(115200);
  delay(2000);
  Serial.println("Starting SSD1306 with the Arduino_GFX library");

  // Set the I2C pin numbers
  Wire.begin(PIN_SDA, PIN_SCL);

  // Call begin() to initialise the display
  // SSD1306_SWITCHCAPVCC = generate display voltage from 3.3V internally
  if(!ssd1306.begin(SSD1306_SWITCHCAPVCC, I2C_ADDRESS)) { 
    Serial.println("Initialisation of SSD1306 failed");
    vTaskSuspend(NULL);
  }

  // Show initial display buffer contents on the screen.
  // The library initializes this with an Adafruit splash screen..
  ssd1306.display();
  delay(2000);
}

//----------------------------------------------------------------------
// loop
// ----
//----------------------------------------------------------------------
void loop() {
  // Clear the screen
  ssd1306.clearDisplay();

  // Draw some text
  ssd1306.setTextColor(WHITE);
  ssd1306.setCursor(0, 0);
  for (int i = 1; i < 3; i++) {
    ssd1306.setTextSize(i);
    ssd1306.println("Some text\n");
  }

  // Draw a frame round the screen
  ssd1306.drawRect(0, 0, SCREEN_WIDTH, SCREEN_HEIGHT, WHITE);

  // On the SSD1306 we need to call display() to send the stuff we've
  // drawn to the screen.
  ssd1306.display();
  delay(5000);
}