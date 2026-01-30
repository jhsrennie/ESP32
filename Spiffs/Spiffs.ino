//----------------------------------------------------------------------
// Spiffs
// ======
// Example of using the ESP32 SPIFFS file system
//----------------------------------------------------------------------
#include <stdio.h>
#include <dirent.h>
#include "esp_spiffs.h"

// This is the directory name used to mount the flash partition. It
// could be any name you want but /spiffs seems to be a common (and
// obvious) choice.
#define MYMOUNT "/spiffs"

//----------------------------------------------------------------------
// help
// ----
// Print instructions
//----------------------------------------------------------------------
void help() {
  Serial.println("Type one of the following commands");
  Serial.println("ls   - list files in SPIFFS file system");
  Serial.println("cat  - print file");
  Serial.println("cre  - create new file");
  Serial.println("rm   - delete file");
  Serial.println("help - show this text");
}

//----------------------------------------------------------------------
// listdir
// -------
// List the contents of the (only) SPIFFS directory
// This is just the standard unix way of listing directory contents
// using the dirent structure.
//----------------------------------------------------------------------
bool listdir(const char *path) {
  struct dirent *entry;
  DIR *dp;

  dp = opendir(path);
  if (!dp) {
    Serial.printf("Cannot open directory %s: error %d\n", path, errno);
    return false;
  }

  int numfiles = 0;
  Serial.printf("Files in directory %s:\n", path);
  while((entry = readdir(dp))) {
    Serial.printf("%s\n", entry->d_name);
    numfiles++;
  }
  if (numfiles == 0)
    Serial.println("No files found");

  closedir(dp);
  return true;
}

//----------------------------------------------------------------------
// cat
// ---
// Print file contents
//----------------------------------------------------------------------
bool cat(const char* filename) {
  Serial.printf("Contents of file %s:\n", filename);
  FILE*f = fopen(filename, "r");
  if (!f) {
    Serial.printf("Failed to open file %s: error %d\n", filename, errno);
    return false;
  }

  char buf[256];
  while (fgets(buf, 256, f))
    Serial.print(buf);
  fclose(f);
  Serial.println("");

  return true;
}

//----------------------------------------------------------------------
// cre
// ---
// Create file
//----------------------------------------------------------------------
bool cre(const char* filename) {
  Serial.printf("Creating file %s:\n", filename);
  FILE* f = fopen(filename, "w");
  if (!f) {
    Serial.printf("Failed to create file %s: error %d\n", filename, errno);
    return false;
  }
  Serial.printf("Created %s, type the file contents. Type . to finish:\n", filename);

  while (true) {
    String s = Serial.readStringUntil(10);
    if (s == ".")
      break;
    fputs(s.c_str(), f);
    fputs("\n", f);
  }
  fclose(f);
  Serial.println("File created\n");

  return true;
}

//----------------------------------------------------------------------
// rm
// --
// Delete file
//----------------------------------------------------------------------
bool rm(const char* filename) {
  Serial.printf("Deleting file %s:\n", filename);

  // Check the file exists
  struct stat info;
  if (stat(filename, &info) != 0) {
    Serial.printf("stat(%s) failed: error %d\n", filename, errno);
    return false;
  }

  if (unlink(filename) != 0) {
    Serial.printf("unlink(%s) failed: %d\n", filename, errno);
    return false;
  }
  Serial.printf("File %s deleted\n", filename);

  return true;
}

//----------------------------------------------------------------------
// fullfilename
// ------------
// If the file does not contain a "/" turn it into a fully qualified
// file name
//----------------------------------------------------------------------
String fullfilename(String filename) {
  String s = filename;
  if (s.indexOf('/') < 0)
    s = "/spiffs/" + s;
  return s;
}

//----------------------------------------------------------------------
// setup
//----------------------------------------------------------------------
void setup() {
  Serial.setTimeout(1000*1000); // Long timeout for reading user input
  Serial.begin(115200);
  delay(2000);
  Serial.println("Starting Spiffs test");

  // Mount the SPIFFS file system
  // This is the only ESP32 specific code. All the file handling uses
  // the same C API you'd use on any OS.
  Serial.println("Mounting the file system");

  esp_vfs_spiffs_conf_t conf = {
    .base_path = MYMOUNT,
    .partition_label = NULL,
    .max_files = 5,
    .format_if_mount_failed = true
  };
  esp_err_t ret = esp_vfs_spiffs_register(&conf);

  if (ret != ESP_OK) {
    if (ret == ESP_FAIL) {
      Serial.println("Failed to mount or format filesystem");
    } else if (ret == ESP_ERR_NOT_FOUND) {
      Serial.println("Failed to find SPIFFS partition");
    } else {
      Serial.printf("Failed to initialize SPIFFS (%s)\n", esp_err_to_name(ret));
    }
    vTaskSuspend(NULL);
  }
  Serial.printf("Mounted %s\n", MYMOUNT);

  // Check the partition
  Serial.println("Checking the file system");
  ret = esp_spiffs_check(conf.partition_label);
  if (ret != ESP_OK) {
    Serial.printf("SPIFFS_check() failed (%s)\n", esp_err_to_name(ret));
    return;
  } else {
    Serial.println("SPIFFS_check() successful");
  }

  // All done
  help();
}

//----------------------------------------------------------------------
// loop
//----------------------------------------------------------------------
void loop() {
  while (true) {
    String s = Serial.readStringUntil(10);
    if (s == "ls") {
      listdir(MYMOUNT);
    }
    else if (s == "cat") {
      Serial.println("Enter file name:");
      s = fullfilename(Serial.readStringUntil(10));
      cat(s.c_str());
    }
    else if (s == "cre") {
      Serial.println("Enter file name to create:");
      s = fullfilename(Serial.readStringUntil(10));
      cre(s.c_str());
    }
    else if (s == "rm") {
      Serial.println("Enter file name to delete:");
      s = fullfilename(Serial.readStringUntil(10));
      rm(s.c_str());
    }
    else if (s == "help") {
      help();
    }
    else {
      Serial.printf("Unknown command %s\n", s.c_str());
    }
  }
}