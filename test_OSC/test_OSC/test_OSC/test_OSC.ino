#include <WiFi.h>
#include <AsyncUDP.h>
#include <OSCMessage.h>

const char* ssid = "AndroidAP46E6";
const char* password = "gregorio";
IPAddress outIp(192, 168, 185, 156);
const unsigned int outPort = 5005;

AsyncUDP udp;
AsyncUDPMessage udpMessage;

int counter = 0;  //Create a variable named counter, that is an integer (whole number) and has a starting value of 0
float f_counter = 0.f;

void setup() {
  Serial.begin(115200);
  WiFi.begin(ssid, password);
  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }
  Serial.println("\nConnected to WiFi");

  if (udp.connect(outIp, outPort)) {
    Serial.println("UDP connected");
    udp.print("/test Hello");
  }
}

void loop() {
  OSCMessage msg("/test");
  msg.add(counter);
  msg.add(2.f);
  msg.add(f_counter);
  Serial.println(counter);
  Serial.println(f_counter);
  counter++;
  f_counter++;

  // AsyncUDPMessage udpMessage;
  msg.send(udpMessage);  // Send OSC message to UDPMessage buffer
  udp.send(udpMessage);  // Send the message over UDP
  msg.send(Serial);      // Send OSC message over Serial to monitor

  // Serial.print("UDP Message Size: ");
  // Serial.println(udpMessage.length());
  msg.empty();  // Free space occupied by message
  udpMessage.flush();

  delay(1000);
}
