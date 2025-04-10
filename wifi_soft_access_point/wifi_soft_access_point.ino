#include <WiFi.h>
#include <WebServer.h>
#include <LittleFS.h>
#include "index.h"

// Uncomment this line when using Debugprobe/Picoprobe
#define UPLOAD_METHOD_DEBUGPROBE

#ifdef UPLOAD_METHOD_DEBUGPROBE
  #define DEBUG_SERIAL Serial1
#else
  #define DEBUG_SERIAL Serial
#endif

const char *ssid = "PicoW_AP"; // AP SSID
const char *password = "12345678"; // AP Password

WebServer server(80);

bool little_started = false;

// Function to read a specific line (0-based) from config.txt
String readConfig(int lineNumber) {
    File file = LittleFS.open("/config.txt", "r");
    if (!file) {
        return "Cannot open config.txt. If this is your first time running the picoBridge, please fill the config and save it.";
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
void saveConfig(String ssid, String pw, String ip, String port) {
    DEBUG_SERIAL.println("Saving config...");
    File file = LittleFS.open("/config.txt", "w");
    if (file) {
        file.println(ssid);
        file.println(pw);
        file.println(ip);
        file.println(port);
        file.close();
        DEBUG_SERIAL.println("Saved successfully.");
    } else {
        DEBUG_SERIAL.println("Failed to open config.txt for writing.");
    }
}

// Serve the main page
void handleRoot() {
  String page = webpage;
  page.replace("%CONFIG_ssid%", readConfig(0));
  page.replace("%CONFIG_pw%",   readConfig(1));
  page.replace("%CONFIG_ip%",   readConfig(2));
  page.replace("%CONFIG_port%", readConfig(3));
  server.send(200, "text/html", page);
}

// Handle saving config
void handleSave() {
    if (server.hasArg("config_ssid") && server.hasArg("config_pw") &&
        server.hasArg("config_ip") && server.hasArg("config_port")) {

        saveConfig(server.arg("config_ssid"),
                   server.arg("config_pw"),
                   server.arg("config_ip"),
                   server.arg("config_port"));
    }

    server.sendHeader("Location", "/");
    server.send(303);
}

// Print to DEBUG_SERIAL
void handlePrint() {
    DEBUG_SERIAL.println("---- Config.txt contents ----");
    DEBUG_SERIAL.println("SSID: " + readConfig(0));
    DEBUG_SERIAL.println("PWD:  " + readConfig(1));
    DEBUG_SERIAL.println("IP:   " + readConfig(2));
    DEBUG_SERIAL.println("PORT: " + readConfig(3));
    DEBUG_SERIAL.println("-----------------------------");

    server.send(200, "text/plain", "Printed to Serial!");
}

void setup() {
  DEBUG_SERIAL.begin(115200);

  while (!DEBUG_SERIAL) yield();
  DEBUG_SERIAL.println("Serial Initialized!");
 
  LittleFSConfig cfg;
  cfg.setAutoFormat(true);
  LittleFS.setConfig(cfg);
  // LittleFS.begin();

  little_started = LittleFS.begin();
  // Start LittleFS
  if (!little_started) {
    DEBUG_SERIAL.println("LittleFS Mount Failed");
    return;
  }

  // Set up Soft AP
  WiFi.softAP(ssid, password);
  DEBUG_SERIAL.println("Access Point started");
  DEBUG_SERIAL.print("AP IP address: ");
  DEBUG_SERIAL.println(WiFi.softAPIP());

  // Web server routes
  server.on("/", handleRoot);
  server.on("/save", HTTP_POST, handleSave);
  server.on("/print", handlePrint);

  // Start server
  server.begin();
  DEBUG_SERIAL.println("HTTP server started");
}

void loop() {
  if (little_started){
    server.handleClient();
  }
}

