#include <Arduino.h>

#define UPLOAD_METHOD_DEBUGPROBE
#define VERBOSE 1 // define verbosity : {0=No_verbose, 1=verbose, 2=very_verbose}

#include "utils.h"   
#include "twomode.h"

TwoMode tmcontroller(Utils::LED_PIN);

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
  
  pinMode(Utils::LED_PIN, OUTPUT);

#if (VERBOSE > 0)
  while (!DEBUG_SERIAL) yield();
  DEBUG_SERIAL.println("\nSerial Initialized!");
#endif

  tmcontroller.init();

  mode = tmcontroller.getMode();
#if (VERBOSE > 0)
  DEBUG_SERIAL.print("Starting Mode ");
  DEBUG_SERIAL.println(mode);
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