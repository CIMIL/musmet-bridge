#ifndef _TWOMODE_H    // Put these two lines at the top of your file.
#define _TWOMODE_H    // (Use a suitable name, usually based on the file name.)


#include <EEPROM.h>

class TwoMode {
public:
  TwoMode(short ledPin) {
    // Initialize LED pin
    this->ledPin = ledPin;
  }

  void init(){
    // Initialize EEPROM
    EEPROM.begin(4);  // Initialize with at least 4 bytes
    // Read the previous flash count from EEPROM
    savedmode = EEPROM.read(EEPROM_ADDR);

    // 5 quick flashes
    for (int i = 0; i < 5; i++) {
      digitalWrite(ledPin, HIGH);
      delay(100);
      digitalWrite(ledPin, LOW);
      delay(100);
    }
    delay(1000);

    for (int i = 0; i < 2; i++) {
      // Turn LED on
      EEPROM.write(EEPROM_ADDR, i+1);
      EEPROM.commit();
      
      digitalWrite(ledPin, HIGH);
      delay(flashDelay);
      
      // Turn LED off
      digitalWrite(ledPin, LOW);
      delay(flashDelay);
    }
  }

  short getMode(){
    return savedmode;
  }
private:
  // Variables for tracking restarts
  int EEPROM_ADDR = 0;  // EEPROM address to store flash count
  unsigned int flashDelay = 1000;  // Time between flashes (ms)
  short savedmode;
  short ledPin;
};

#endif // _TWOMODE_H    // Put this line at the end of your file.