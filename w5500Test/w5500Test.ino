//**********************************************************************
// Test sketch for the W5500 ethernet module
// C3 SPI pins
// MOSI: 6
// MISO: 5
// SCK: 4
// SS: 7
//**********************************************************************
#include <SPI.h>
#include <Ethernet.h>

// The MAC address of the w5500 needs to be supplied
// You can make up any random address
byte mac[] = { 0xAA, 0xBB, 0xCC, 0xDD, 0xEE, 0xFF };

// Print the SPI pin IDs
void PrintSPIPins() {
  Serial.println("SPI pins");
  Serial.printf("MOSI: %d\n", MOSI);
  Serial.printf("MISO: %d\n", MISO);
  Serial.printf("SCK:  %d\n", SCK);
  Serial.printf("SS:   %d\n", SS);  
}

// setup
void setup() {
  Serial.begin(115200);

  // Print the pin IDs
  PrintSPIPins();

  // We need to specify the SS pin being used
  Ethernet.init(SS);
  // initialize the Ethernet shield using DHCP:
  if (Ethernet.begin(mac) == 0) {
    Serial.println("Initialisation failed");

    // check for Ethernet hardware present
    if (Ethernet.hardwareStatus() == EthernetNoHardware)
      Serial.println("Ethernet shield was not found");

    // check for Ethernet cable
    if (Ethernet.linkStatus() == LinkOFF)
      Serial.println("Ethernet cable is not connected.");

    // Give up at this point
    return;
  }

  // Print the details
  Serial.printf("Ethernet chip ID = %d\n", Ethernet.hardwareStatus());
  Serial.print("Ethernet initialised, IP address = ");
  Serial.println(Ethernet.localIP());
}

void loop() {
  delay(10000);
}
