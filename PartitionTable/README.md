# PartitionTable

If you are using the Arduino IDE there is almost never any reason to mess with the partition setting as the default setting usually works fine. However I recently needed to set up a large SPIFFS partition on an S3 N16R8 that has 16B of flash, and the Arduino IDE has no way of doing this. If you find yourself in a similar situation this sketch demonstrates how it can be done.

In the Arduino IDE you normally choose the partition layout by opening the _Tools_ menu, clcking _Partition Scheme_ and choosing one of the predefined options. If you need a scheme that is not predefined then you need to create a file called `partitions.csv` with the partition definitions and copy this file into the directory containing your sketch. Then click _Tools_, _Partition Scheme_ and choose _Custom_. The IDE will then flash the ESP32 with the partition scheme defined in the file.

The file included here creates a 1MB app partition and a 12MB SPIFFS partition. The file contains:

```
# Name,   Type, SubType, Offset,  Size, Flags
nvs,      data, nvs,     0x9000,  0x5000,
otadata,  data, ota,     0xe000,  0x2000,
app0,     app,  ota_0,   0x10000, 0x100000,
app1,     app,  ota_1,           ,0x100000,
spiffs,   data, spiffs,          ,0xC00000,
```

There are two identically sized `app` partitions to allow for OTA updates, and then the SPIFFS partition. `0x100000` is 1MB in hex so the app partition(s) is 1MB and the SPIFFs partition 12MB. You can mess around with the sizes as long as the total size is equal to or less than the size of the flash in your ESP32. Don't mess around with he first two partitions!

If yu run this on an S3 N16R8 like mine remember to use _Tools_, _Flash size_ to set the amount of flash memory to 16MB.