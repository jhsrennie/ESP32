# esp_new_jpeg

Espressif provide a library for their ESP-IDF toolchain called ESP_NEW_JPEG that contains functions to encode and decode JPEG images. It can be downloaded from [the Espressif component registry here](https://components.espressif.com/components/espressif/esp_new_jpeg/). The library works very well but it cannot be used with the Arduino IDE. This project adapts the library for use with the Arduino IDE and includes a couple of sketches showing how the library can be used to encode images.

This project contains the following:

- esp_new_jpeg: the Arduino IDE library

- ServeJPG: a sketch that uses the library to convert bitmaps to JPEG images that can be downloaded from a web server

- StreamJPG: a sketch that works like  webcam. It continuously converts bitmaps to JPEG and streams them as a webcam would

See the Readme files in the three directories for more details.

This is version 1.0.2. If you need to update to a newer version there are details below.

### Installation

The libraries used by the IDE are stored in your _Sketchbook location_ directory. To locate this directory open the IDE and click _File_, _Preferences_ and look at the _Sketchbook location_ field. If you look in this directory there will be a directory called _libraries_, and in the _libraries_ directory there will be a directory for each library. You need to copy this directory into the _libraries_ folder so the directory structure looks like this:

```
Sketchbook location
 |_ libraries
     |_ esp_new_jpeg
```
Once you have copied the directory the library will be available to use, though note that it doe not appear in the IDE libraries listing (I am not sure why not).

### Updating the library

The version included here is v1.0.2. I will keep updating my version whenever I notice Espressif have released a new version but this is something you can also do yourself. If you need to update to a later version this has to be done by downloading the new version from the Espressif component registry and manually copying in the new files.

Download the zip from [the component registry](https://components.espressif.com/components/espressif/esp_new_jpeg/) and unzip it to some convenient temporary location. When unzipped the directory structure will look like:

```
esp_new_jpeg
|_include
|_lib
| |_esp32
| !_esp32c2
| |_etc
|_ test app
```

And the directory structure of the Arduino library is:

```
esp_new_jpeg
|_src
  |_esp32
  !_esp32c2
  |_etc  
```

To update the Arduino library copy all the `.h` files from the component `include` directory into the Arduino `src` directory, then copy all the subdirectories from the component `lib` directory into the Arduino `src` directory. You'll get warnings about replacing the existing files and that's fine because that's exactly what you need to do to update the library.

### How it works

Adapting the component for the Arduino IDE turned out to be very simple. The component distributes the JPEG library as precompiled binaries not as source code. That's why there are separate subdirectories `esp32`, `esp32c2`, etc because there is a different binary for each type of CPU. If you look in the directories for the different CPUs you'll see they all contain the library file `libesp_new_jpeg.a`. There are different versions because the CPUs have different capabilities. For example the S3 has SIMD parallel instructions that greatly speed up algorithms like JPEG encoding.

Anyhow, [the Arduino IDE library specification](https://docs.arduino.cc/arduino-cli/library-specification/?hl=en-GB#precompiled-binaries) supports precompiled binaries. You just need to add `precompiled=true` to the `library.properties` file. All I had to do was created a library and copy in the headers and binaries as described in the _Updating the library_ section. I was a bit surprised that this "just worked", but it does!