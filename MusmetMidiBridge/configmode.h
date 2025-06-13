
#ifndef _CONFIGMODE_H
#define _CONFIGMODE_H

#include <WiFi.h>
#include <WebServer.h>
#include <DNSServer.h>
#include "index.h" // HTML main configuration page
#include "saved.h" // HTML saved page
#include "utils.h" // Read WiFi config from the filesystem


const char *PICO_SSID = "PicoW_BRIDGE_Config"; // AP SSID
const char *PICO_PASSWORD = "picopico"; // AP Password
const char *TARGET_HOSTNAME = "bridge.conf";  


class ConfigMode { 
  
  const byte DNS_PORT = 53;
  DNSServer dnsServer;
  WebServer server;

  // Redirect to "bridge.conf" 
  void redirectToHostname() {
        server.sendHeader("Location", "http://" + String(TARGET_HOSTNAME) + "/", true);
        server.send(302, "text/html", "<html><body>Redirecting...</body></html>");
  }

  // Check if actual hostHeader is "bridge.conf"
  void checkHostHeader() {
        if (server.hostHeader() != TARGET_HOSTNAME) {
#ifdef VERBOSE
    DEBUG_SERIAL.print("Redirecting...");
    DEBUG_SERIAL.println(server.hostHeader());
#endif
            redirectToHostname();
            return;
        }
    }
  
  // Serve the main page
  void handleRoot() {
#ifdef VERBOSE
    DEBUG_SERIAL.println("Serving root page.");
#endif
    checkHostHeader(); 

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
    
    checkHostHeader();

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
    checkHostHeader();
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
    WiFi.softAP(PICO_SSID, PICO_PASSWORD);
#ifdef VERBOSE
    DEBUG_SERIAL.println("Access Point started");
    DEBUG_SERIAL.print("AP IP address: ");
    DEBUG_SERIAL.println(WiFi.softAPIP());
    DEBUG_SERIAL.println(WiFi.getHostname());
#endif
  
    // Start DNS server to redirect all domains to our IP
    dnsServer.start(DNS_PORT, "*", WiFi.softAPIP());
  
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
    server.onNotFound([this]() { 
            this->redirectToHostname(); 
        });
  
    // Start server
    server.begin();
#ifdef VERBOSE
    DEBUG_SERIAL.println("HTTP server started");
#endif
  }
      
  void loop() {
    if (Utils::little_started){
      dnsServer.processNextRequest();
      server.handleClient();
      Utils::blinkLED(0);
    }
  }
};

#endif // _CONFIGMODE_H





