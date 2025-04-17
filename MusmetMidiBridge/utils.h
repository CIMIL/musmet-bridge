#ifndef _UTILS_H 
#define _UTILS_H


#ifdef VERBOSE
  #ifdef UPLOAD_METHOD_DEBUGPROBE
    #define DEBUG_SERIAL Serial1
  #else
    #define DEBUG_SERIAL Serial
  #endif
#endif


#include <LittleFS.h> // To read Wifi config from the filesystem
#include <WiFi.h>

namespace Utils {

bool little_started = false;
void startLittleFS() {
  if (!little_started) {
    LittleFSConfig cfg;
    cfg.setAutoFormat(true);
    LittleFS.setConfig(cfg);

    little_started = LittleFS.begin();
    // Start LittleFS
    if (!little_started) {
#ifdef VERBOSE
      DEBUG_SERIAL.println("LittleFS Mount Failed");
#endif
      return;
    }
  }
}

enum ConfigLine {
  SSID = 0,
  PASSWORD,
  IP,
  PORT
};


// Function to read a specific line (0-based) from config.txt
String readWiFiConfig(int lineNumber) {
  File file = LittleFS.open("/config.txt", "r");
  if (!file) {
      return "Cannot open config.txt. If this is your first time running the Bridge, please fill the config and save it.";
  }

  String line;
  int currentLine = 0;
  while (file.available()) {
    line = file.readStringUntil('\n');
    line.trim(); // remove any \r or whitespace
    if (currentLine == lineNumber) {
      file.close();
      return line;
    }
    currentLine++;
  }

  file.close();
  return "";  // Not enough lines
}

// Save config values to file
void saveWiFiConfig(String ssid, String pw, String ip, String port) {
#ifdef VERBOSE
  DEBUG_SERIAL.println("Saving config...");
#endif
  File file = LittleFS.open("/config.txt", "w");
  if (file) {
    file.println(ssid);
    file.println(pw);
    file.println(ip);
    file.println(port);
    file.close();
#ifdef VERBOSE
    DEBUG_SERIAL.println("Saved successfully.");
#endif
  } else {
#ifdef VERBOSE
    DEBUG_SERIAL.println("Failed to open config.txt for writing.");
#endif
  }
}


// List available WiFi options
String getNetworkOptions(){
  String options;
  String savedSSID = readWiFiConfig(SSID);  // Read saved SSID from config.txt

  int numNetworks = WiFi.scanNetworks();
  for (int i = 0; i < numNetworks; i++) {
    String currentSSID = WiFi.SSID(i);
    options += "<option value='" + currentSSID + "'";
    if (currentSSID == savedSSID) {
      options += " selected";
    }
    options += ">" + currentSSID + "</option>";
  }

  return options;
}

} // namespace Utils


#endif // _UTILS_H 