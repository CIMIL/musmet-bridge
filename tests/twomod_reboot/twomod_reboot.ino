#include <Arduino.h>

#define UPLOAD_METHOD_DEBUGPROBE
#include "utils.h"   // Defines DEBUG_SERIAL
#include "twomode.h"

const short LED_PIN = 15;

TwoMode tmcontroller(LED_PIN);
void setup() {
  // Initialize serial connection
  DEBUG_SERIAL.begin(115200);
  
  pinMode(LED_PIN, OUTPUT);

  while (!DEBUG_SERIAL) yield();
  DEBUG_SERIAL.println("\nSerial Initialized!");

  tmcontroller.init();

  short mode = tmcontroller.getMode();
  DEBUG_SERIAL.print("Starting Mode ");
  DEBUG_SERIAL.print(mode);
}

void loop() {
}