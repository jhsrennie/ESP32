Spiffs
======
Example of using the ESP32 SPIFFS file system.

The ESP32 can divide the flash memory into two parts - one for your code and the other for storing files in the same way an SD card stores files. This sketch is a simple illustration of how to use SPIFFS to create files, read iles, do directory listings and delete files.

The file handling is actually just done with the same C functions that you'd use on any operating system. The only ESP specific code is a single function call, esp_vfs_spiffs_register(), to mount the flash partition used for stoeing files.