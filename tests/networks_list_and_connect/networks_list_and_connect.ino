#include <WiFi.h>

bool isConnected = false;

const char *macToString(uint8_t mac[6]) {
  static char s[20];
  sprintf(s, "%02X:%02X:%02X:%02X:%02X:%02X", mac[0], mac[1], mac[2], mac[3], mac[4], mac[5]);
  return s;
}

const char *encToString(uint8_t enc) {
  switch (enc) {
    case ENC_TYPE_NONE: return "NONE";
    case ENC_TYPE_TKIP: return "WPA";
    case ENC_TYPE_CCMP: return "WPA2";
    case ENC_TYPE_AUTO: return "AUTO";
  }
  return "UNKN";
}

void setup() {
  Serial.begin(115200);
  while (!Serial) yield();
  Serial.println("Welcome to test sketch...");
  WiFi.mode(WIFI_STA);
  
}

void handleNetworkSelection(int cnt) {
  Serial.print("\nEnter network number (0-");
  Serial.print(cnt-1);
  Serial.println(") or -1 to skip:");
  
  while (!Serial.available());
  int selection = Serial.parseInt();
  Serial.readString(); // Clear buffer

  if (selection == -1) return;
  if (selection < 0 || selection >= cnt) {
    Serial.println("Invalid selection!");
    return;
  }

  String ssid = WiFi.SSID(selection);
  Serial.print("Enter password for '");
  Serial.print(ssid);
  Serial.println("': ");
  
  while (!Serial.available());
  String password = Serial.readStringUntil('\n');
  password.trim();

  Serial.print("\nConnecting to ");
  Serial.println(ssid);
  WiFi.begin(ssid.c_str(), password.c_str());

  unsigned long start = millis();
  while (WiFi.status() != WL_CONNECTED && millis() - start < 20000) {
    delay(500);
    Serial.print(".");
  }

  if (WiFi.status() == WL_CONNECTED) {
    isConnected = true;
    Serial.println("\nConnected!");
    Serial.print("IP address: ");
    Serial.println(WiFi.localIP());
  } else {
    Serial.println("\nConnection failed!");
    WiFi.disconnect();
  }
}

void loop() {
  if (!isConnected) {
    delay(5000);
    Serial.printf("\nScanning networks at %lu...\n", millis());
    
    int cnt = WiFi.scanNetworks();
    if (!cnt) {
      Serial.println("No networks found");
    } else {
      Serial.printf("%32s %5s %17s %2s %4s\n", "SSID", "ENC", "BSSID        ", "CH", "RSSI");
      for (int i=0; i<cnt; i++) {
        uint8_t bssid[6];
        WiFi.BSSID(i, bssid);
        Serial.printf("%32s %5s %17s %2d %4ld\n", 
          WiFi.SSID(i), 
          encToString(WiFi.encryptionType(i)), 
          macToString(bssid), 
          WiFi.channel(i), 
          WiFi.RSSI(i)
        );
      }
      handleNetworkSelection(cnt);
    }
  } else {
    // Maintain connection
    if (WiFi.status() != WL_CONNECTED) {
      isConnected = false;
      Serial.println("Connection lost!");
    }
    delay(5000);
  }
}
