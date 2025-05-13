#include <Arduino.h>

#define VERBOSE
#define UPLOAD_METHOD_DEBUGPROBE
#include "utils.h"   
#include "twomode.h"

const short LED_PIN = LED_BUILTIN;
TwoMode tmcontroller(LED_PIN);

#include "oscmode.h"
#include "configmode.h"

enum Modes {
  NONE = 0,
  CONFIG_MODE = 1,
  OSC_MODE = 2
};
short mode;


OscMode oscmode;
ConfigMode configmode;

void setup() {
  // Initialize serial connection
#ifdef VERBOSE
  DEBUG_SERIAL.begin(115200);
#endif
  
  pinMode(LED_PIN, OUTPUT);

#ifdef VERBOSE
  while (!DEBUG_SERIAL) yield();
  DEBUG_SERIAL.println("\nSerial Initialized!");
#endif

  tmcontroller.init();

  mode = tmcontroller.getMode();
#ifdef VERBOSE
  DEBUG_SERIAL.print("Starting Mode ");
  DEBUG_SERIAL.print(mode);
#endif

  if (mode == OSC_MODE)
  {
    oscmode.setup();
  } else if (mode == CONFIG_MODE) {
    configmode.setup();
  } else {

  }
}

void loop() {
  if (mode == OSC_MODE)
  {
    oscmode.loop();
  } else if (mode == CONFIG_MODE) {
    configmode.loop();
  } else {

  }
}