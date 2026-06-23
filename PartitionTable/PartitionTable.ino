//----------------------------------------------------------------------
// PartitionTable
// ==============
// This demonstrates how to create your own partition table.
// You simply write a partitions.csv file and drop it into the same
// directory as your .ino file.
// This example creates a 1MB app partition (with OTA) and a 12MB
// spiffs partition that is used for a LittleFS file system. It requires
// 16MB of flash so you need something like an S3 N16R8.
//----------------------------------------------------------------------
#include <FS.h>
#include <LittleFS.h>
#include <esp_littlefs.h>

#define FORMAT_LITTLEFS_IF_FAILED true

//----------------------------------------------------------------------
// ESPInfo
// -------
// Display info about the CPU
//----------------------------------------------------------------------
void ESPInfo() {
  Serial.println("=== ESP32 INFO ===");
  Serial.printf("Chip model: %s\n", ESP.getChipModel());
  Serial.printf("Cores: %d\n", ESP.getChipCores());
  Serial.printf("Chip revision: %d\n", ESP.getChipRevision());
  Serial.printf("CPU frequency: %d MHz\n", getCpuFrequencyMhz());

  Serial.println("\n=== MEMORY ===");
  Serial.printf("Total internal RAM: %d bytes\n", ESP.getHeapSize());
  Serial.printf("Free internal RAM: %d bytes\n", ESP.getFreeHeap());
  Serial.printf("Total PSRAM: %d bytes\n", ESP.getPsramSize());
  Serial.printf("Free PSRAM: %d bytes\n", ESP.getFreePsram());

  Serial.println("\n=== FLASH ===");
  Serial.printf("Total flash: %d bytes\n", ESP.getFlashChipSize());
  Serial.printf("Flash speed: %d Hz\n", ESP.getFlashChipSpeed());

  Serial.println("\n=== SDK / CORE ===");
  Serial.printf("SDK version: %s\n", ESP.getSdkVersion());
  Serial.printf("Arduino Core version: %d.%d.%d\n",
  ESP_ARDUINO_VERSION_MAJOR,
  ESP_ARDUINO_VERSION_MINOR,
  ESP_ARDUINO_VERSION_PATCH);
}

//----------------------------------------------------------------------
// setup
// -----
//----------------------------------------------------------------------
void setup(){
  Serial.begin(115200);
  delay(2000);
  ESPInfo();

  // Mount the LittleFS partition
  if(!LittleFS.begin(FORMAT_LITTLEFS_IF_FAILED)){
    Serial.println("LittleFS Mount Failed");
    return;
  }

  // Get the partition size
  // Note that LittleFS.begin uses "spiffs" as the partitoon name by default
  size_t total = 0, used = 0;
  esp_err_t ret = esp_littlefs_info("spiffs", &total, &used);
  if (ret != ESP_OK) {
    Serial.printf("\nFailed to get LittleFS partition information (%s)\n", esp_err_to_name(ret));
  }
  else {
    Serial.printf("\nPartition size: total: %d, used: %d\n", total, used);
  }
}

//----------------------------------------------------------------------
// loop
// ----
// Not used
//----------------------------------------------------------------------
void loop(){
  delay(1000);
}