#ifndef _CONFIGMODE_H
#define _CONFIGMODE_H

#include <WiFi.h>
#include <WebServer.h>
#include "index.h" // HTML main page
#include "saved.h" // HTML saved page
#include "utils.h" // Read WiFi config from the filesystem


const char *PICO_SSID = "PicoW_BRIDGE_Config"; // AP SSID
const char *PICO_PASSWORD = "picopico"; // AP Password

class ConfigMode { 
  
  // const byte DNS_PORT = 53;
  // DNSServer dnsServer;
  
  WebServer server;
  
  
  // Serve the main page
  void handleRoot() {
#ifdef VERBOSE
    DEBUG_SERIAL.println("Serving root page.");
#endif
    String page = webpage;
    page.replace("%NETWORK_OPTIONS%", Utils::getNetworkOptions());
    //page.replace("%CONFIG_ssid%", Utils::readWiFiConfig(Utils::ConfigLine::SSID));
    page.replace("%CONFIG_pw%",   Utils::readWiFiConfig(Utils::ConfigLine::PASSWORD));
    page.replace("%CONFIG_ip%",   Utils::readWiFiConfig(Utils::ConfigLine::IP));
    page.replace("%CONFIG_port%", Utils::readWiFiConfig(Utils::ConfigLine::PORT));
    server.send(200, "text/html", page);
  }
  
  // Handle saving config
  void handleSave() {
    if (server.hasArg("config_ssid") && server.hasArg("config_pw") &&
      server.hasArg("config_ip") && server.hasArg("config_port")) {
  
      Utils::saveWiFiConfig(server.arg("config_ssid"),
                  server.arg("config_pw"),
                  server.arg("config_ip"),
                  server.arg("config_port"));
    }

#ifdef VERBOSE
    DEBUG_SERIAL.println("Serving saved page.");
#endif
    String spage = savedPage;
    spage.replace("%CONFIG_ssid%", Utils::readWiFiConfig(Utils::ConfigLine::SSID));
    spage.replace("%CONFIG_pw%",   Utils::readWiFiConfig(Utils::ConfigLine::PASSWORD));
    spage.replace("%CONFIG_ip%",   Utils::readWiFiConfig(Utils::ConfigLine::IP));
    spage.replace("%CONFIG_port%", Utils::readWiFiConfig(Utils::ConfigLine::PORT));
    server.send(200, "text/html", spage);
  }
  
  // Print to DEBUG_SERIAL
  void handlePrint() {
#ifdef VERBOSE
    DEBUG_SERIAL.println("---- Config.txt contents ----");
    DEBUG_SERIAL.println("SSID: " + Utils::readWiFiConfig(Utils::ConfigLine::SSID));
    DEBUG_SERIAL.println("PWD:  " + Utils::readWiFiConfig(Utils::ConfigLine::PASSWORD));
    DEBUG_SERIAL.println("IP:   " + Utils::readWiFiConfig(Utils::ConfigLine::IP));
    DEBUG_SERIAL.println("PORT: " + Utils::readWiFiConfig(Utils::ConfigLine::PORT));
    DEBUG_SERIAL.println("-----------------------------");
#endif
  
    server.send(200, "text/plain", "Printed to Serial!");
  }
public:

  ConfigMode() : server(80){

  }

  void setup() {
#ifdef VERBOSE
    while (!DEBUG_SERIAL) yield();
#endif
    
    Utils::startLittleFS(); // Start LittleFS
  
    // Set up Soft AP
    // WiFi.setHostname("picobridge");
    WiFi.softAP(PICO_SSID, PICO_PASSWORD);
#ifdef VERBOSE
    DEBUG_SERIAL.println("Access Point started");
    DEBUG_SERIAL.print("AP IP address: ");
    DEBUG_SERIAL.println(WiFi.softAPIP());
#endif
    // DEBUG_SERIAL.println(WiFi.getHostname());
  
    // Start DNS server to redirect all domains to our IP
    // dnsServer.start(DNS_PORT, "*", WiFi.softAPIP());
  
    // Web server routes
    server.on("/", [this]() { 
            this->handleRoot(); 
        });
    server.on("/save", HTTP_POST, [this]() { 
            this->handleSave(); 
        });
    server.on("/print", [this]() { 
            this->handlePrint(); 
        });
  
    // Start server
    server.begin();
#ifdef VERBOSE
    DEBUG_SERIAL.println("HTTP server started");
#endif
  }
      
  void loop() {
    if (Utils::little_started){
      // dnsServer.processNextRequest();
      server.handleClient();
    }
  }
};

#endif // _CONFIGMODE_H





