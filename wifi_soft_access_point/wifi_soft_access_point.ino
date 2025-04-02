#include <WiFi.h>
#include <WebServer.h>
#include <LittleFS.h>

const char *ssid = "PicoW_AP"; // AP SSID
const char *password = "12345678"; // AP Password

WebServer server(80);

bool little_started = false;

// Function to read the config file
String readConfig(int propIndex) {
    File file = LittleFS.open("/config.txt", "r");
    if (!file) {
        return "Error: Cannot open config.txt";
    }
    String content = file.readString();
    file.close();

    // TO DEBUG THIS
    int prevPtr = 0;
    int endstrPtr = content.indexOf('\n', 0);
    while (endstrPtr > -1 && propIndex >= 0) {
      if (propIndex == 0)
      {
        int start = prevPtr;
        int end = endstrPtr;
        return content.substring(endstrPtr,end);
      }
      propIndex--;
      prevPtr = endstrPtr+1;
      endstrPtr = content.indexOf('\n', prevPtr);
    }
    return "";
}

// Function to save values to config file
void saveConfig(String ssid, String pw, String ip, String port) {
    Serial.println("SSid is"+ssid);
    File file = LittleFS.open("/config.txt", "w");
    if (file) {
        file.println(ssid);
        file.println(pw);
        file.println(ip);
        file.print(port);
        file.close();
    }
}

// HTML webpage
const char webpage[] PROGMEM = R"rawliteral(
<!DOCTYPE html>
<html>
<head>
    <title>PicoW Config</title>
</head>
<body>
    <h2>Modify Config.txt</h2>
    <form action="/save" method="POST">
        <p>SSID:</p>
        <textarea name="config_ssid" rows="1" cols="30">%CONFIG_ssid%</textarea><br>
        <p>PWD: </p>
        <textarea name="config_pw"   rows="1" cols="30">%CONFIG_pw%</textarea><br>
        <p>IP:  </p>
        <textarea name="config_ip"   rows="1" cols="30">%CONFIG_ip%</textarea><br>
        <p>PORT:</p>
        <textarea name="config_port" rows="1" cols="30">%CONFIG_port%</textarea><br>
        <input type="submit" value="Save">
    </form>
    <button onclick="fetch('/print').then(response => alert('Printed to Serial!'));">Print Config</button>
</body>
</html>
)rawliteral";

// Serve the main page
void handleRoot() {
  String page = webpage;
  page.replace("%CONFIG_ssid%", readConfig(0));
  page.replace("%CONFIG_pw%",   readConfig(1));
  page.replace("%CONFIG_ip%",   readConfig(2));
  page.replace("%CONFIG_port%", readConfig(3));
  server.send(200, "text/html", page);
}

// Handle saving new config
void handleSave() {
  Serial.println("Saving");
  
  Serial.println(String("server.hasArg(\"config_ssid\")")+String(server.hasArg("config_ssid")?"true":"false"));
  Serial.println(String("server.hasArg(\"config_pw\")")+ String(server.hasArg("config_pw")?"true":"false"));
  Serial.println(String("server.hasArg(\"config_ip\")")+ String(server.hasArg("config_ip")?"true":"false"));
  Serial.println(String("server.hasArg(\"config_port\")")+ String(server.hasArg("config_port")?"true":"false"));

  if (server.hasArg("config_ssid") && server.hasArg("config_pw") && server.hasArg("config_ip") && server.hasArg("config_port")) {
    saveConfig(server.arg("config_ssid"), server.arg("config_pw"), server.arg("config_ip"), server.arg("config_port"));
  }
  server.sendHeader("Location", "/");
  server.send(303);
}

// Handle printing to Serial
void handlePrint() {
  Serial.println("Config.txt contents:");
  Serial.println("SSID: "+readConfig(0));
  Serial.println("PWD:  "+readConfig(2));
  Serial.println("IP:   "+readConfig(3));
  Serial.println("PORT: "+readConfig(4));
  server.send(200, "text/plain", "Printed to Serial!");
}

void setup() {
  Serial.begin(115200);

  while (!Serial) yield();
  // Serial.println("Ciao Domenico!");

  delay(500);

  LittleFSConfig cfg;
  cfg.setAutoFormat(true);
  LittleFS.setConfig(cfg);
  // LittleFS.begin();

  little_started = LittleFS.begin();
  // Start LittleFS
  if (!little_started) {
    Serial.println("LittleFS Mount Failed");
    return;
  }

  // Set up Soft AP
  WiFi.softAP(ssid, password);
  Serial.print("AP IP address: ");
  Serial.println(WiFi.softAPIP());

  // Web server routes
  server.on("/", handleRoot);
  server.on("/save", HTTP_POST, handleSave);
  server.on("/print", handlePrint);

  // Start server
  server.begin();
}

void loop() {
  if (little_started){
    server.handleClient();
    // Serial.println(ssid);
    // delay(1000);
  }
}

