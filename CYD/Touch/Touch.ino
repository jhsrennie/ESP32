// ---------------------------------------------------------------------
// Touch
// =====
// Test program to see how the touchscreen works on a CYD
// The XPT2046 returns both x and y coordinates in an approximate range
// of 200 to 4000. You need to scale these to the actual screen size.
// ---------------------------------------------------------------------
#include <XPT2046_Touchscreen.h>

// Touchscreen pins
#define XPT2046_IRQ  36  // T_IRQ
#define XPT2046_MOSI 32  // T_DIN
#define XPT2046_MISO 39  // T_OUT
#define XPT2046_CLK  25  // T_CLK
#define XPT2046_CS   33  // T_CS

// Create the global touchscreen object
SPIClass touchscreenSPI = SPIClass(VSPI);
// If you are using your own interrupt handler set the second arg to 255
// to disable the interrupt handling built into the library.
// XPT2046_Touchscreen touchscreen(XPT2046_CS, 255);
XPT2046_Touchscreen touchscreen(XPT2046_CS, XPT2046_IRQ);

// Size of the CYD screen
#define SCREEN_WIDTH  240
#define SCREEN_HEIGHT 320

// Min and max touch points
uint16_t min_x = 4095, min_y = 4095, max_x = 0, max_y = 0;

// Variable used by interrupt
volatile int irqcount = 0;

// Touch interrupt function
void IRAM_ATTR TouchIRQ() {
  irqcount++;
}

// Get the Touchscreen data
void touchscreen_read() {
  // Checks if touchscreen was touched
  if(touchscreen.tirqTouched() && touchscreen.touched()) {
    // Get Touchscreen points
    uint16_t x, y;
    uint8_t pressure;
    touchscreen.readData(&x, &y, &pressure);
    // Or you can do
    TS_Point p = touchscreen.getPoint();

    // Check the limits
    if (min_x > x) min_x = x;
    if (min_y > y) min_y = y;
    if (max_x < x) max_x = x;
    if (max_y < y) max_y = y;

    Serial.printf("Touch: (%4d, %4d) %d\n", x, y, pressure);
    Serial.printf("(%4d, %4d) to (%4d, %4d)\n", min_x, min_y, max_x, max_y);
  }
}

void setup() {
  Serial.begin(115200);
  delay(1000);
  Serial.println("Started CYD touch test");

  // Start the SPI for the touchscreen and init the touchscreen
  touchscreenSPI.begin(XPT2046_CLK, XPT2046_MISO, XPT2046_MOSI, XPT2046_CS);
  touchscreen.begin(touchscreenSPI);

  // You may need to change the screen rotation to put (0,0) at the top left
  // 0 = origin at top right
  // 1 - origin at bottom right
  // 2 - origin at bottom left, axes swapped
  // 3 - origin at top left
  touchscreen.setRotation(3);

  // This configures the interrupt if you want to use it
  // DO NOT call any screen functions in the interrupt function
  // attachInterrupt(digitalPinToInterrupt(XPT2046_IRQ), TouchIRQ, FALLING);
}

void loop() {
  touchscreen_read();
  delay(100);
}