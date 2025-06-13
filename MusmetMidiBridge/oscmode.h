#ifndef _OSCMODE_H
#define _OSCMODE_H

#if !defined(USE_TINYUSB_HOST) || !defined(USE_TINYUSB)
#error "Please use the Menu to select Tools->USB Stack: Adafruit TinyUSB Host"
#else
#warning "All Serial Monitor Output is on DEBUG_SERIAL."
#endif

// USB MIDI Host
#include <EZ_USB_MIDI_HOST.h>

// WiFi/OSC Headers
#include <WiFi.h>
#include <AsyncUDP.h>
#include <OSCMessage.h>
// Read WiFi config from the filesystem
#include "utils.h"


// WIFI Config (to be read from LittleFS)
char cconfigSSID[64], cconfigPWD[64], cOutIP[20], cOutPort[20];

// UDP Client object
AsyncUDP udp;
AsyncUDPMessage udpMessage;

// USB Host object
Adafruit_USBH_Host USBHost;
USING_NAMESPACE_MIDI
USING_NAMESPACE_EZ_USB_MIDI_HOST

// Instantiate the EZ_USB_MIDI_HOST object with name "usbhMIDI" using the default "MidiHostSettingsDefault" configuration
RPPICOMIDI_EZ_USB_MIDI_HOST_INSTANCE(usbhMIDI, MidiHostSettingsDefault)

static uint8_t connectedDevAddrs[RPPICOMIDI_TUH_MIDI_MAX_DEV];
static uint8_t numConnectedDevices = 0;


// Send an OSC message with the given address and arguments 
template <typename... Args>
inline void sendOSCMessage(const char* address, Args... args) {
  OSCMessage msg(address); // Create OSC message
  (msg.add(args), ...);  // Fold expression to add all arguments
  msg.send(udpMessage);  // Send OSC message to UDPMessage buffer
  udp.send(udpMessage);  // Send the message over UDP
  msg.empty();           // Free space occupied by message
  udpMessage.flush();    
}

/* MIDI IN MESSAGE MAPPING */
static void onMidiError(int8_t errCode) {
#if (VERBOSE > 0)
  DEBUG_SERIAL.printf("MIDI Errors: %s %s %s\r\n", (errCode & (1UL << ErrorParse)) ? "Parse" : "",
                      (errCode & (1UL << ErrorActiveSensingTimeout)) ? "Active Sensing Timeout" : "",
                      (errCode & (1UL << WarningSplitSysEx)) ? "Split SysEx" : "");
#endif
}

static void onNoteOff(Channel channel, byte note, byte velocity) {
#if (VERBOSE > 0)
  DEBUG_SERIAL.printf("C%u: Note off#%u v=%u\r\n", channel, note, velocity);
#endif
  digitalWrite(Utils::LED_PIN, LOW);
  char address[12];
  snprintf(address, sizeof(address), "/ch%dnoteoff", (int)channel);
  sendOSCMessage(address, note);  // /ch<channel>noteoff (<note>,)
  snprintf(address, sizeof(address), "/ch%dnoteoffvalue", (int)channel);
  sendOSCMessage(address, velocity);  // /ch<channel>noteoffvalue (<velocity>,)
}

static void onNoteOn(Channel channel, byte note, byte velocity) {
#if (VERBOSE > 0)
  DEBUG_SERIAL.printf("C%u: Note on#%u v=%u\r\n", channel, note, velocity);
#endif
  digitalWrite(Utils::LED_PIN, LOW);
  char address[12];
  snprintf(address, sizeof(address), "/ch%dnote", (int)channel);
  sendOSCMessage(address, note);  // /ch<channel>note (<note>,)
  snprintf(address, sizeof(address), "/ch%dnvalue", (int)channel);
  sendOSCMessage(address, velocity);  // /ch<channel>nvalue (<velocity>,)
}

static void onControlChange(Channel channel, byte controller, byte value) {
#if (VERBOSE > 0)
  DEBUG_SERIAL.printf("C%u: CC#%u=%u\r\n", channel, controller, value);
#endif  
  digitalWrite(Utils::LED_PIN, LOW);
  char address[12];
  snprintf(address, sizeof(address), "/ch%dcc", (int)channel);
  sendOSCMessage(address, controller);  // /ch<channel>cc (<controller>,)
  snprintf(address, sizeof(address), "/ch%dccvalue", (int)channel);
  sendOSCMessage(address, value);  // /ch<channel>ccvalue (<value>,)
}

static void onProgramChange(Channel channel, byte program) {
#if (VERBOSE > 0)
  DEBUG_SERIAL.printf("C%u: Prog=%u\r\n", channel, program);
#endif
}

static void onPitchBend(Channel channel, int value) {
#if (VERBOSE > 0)
  DEBUG_SERIAL.printf("C%u: PB=%d\r\n", channel, value);
#endif
  char address[12];
  snprintf(address, sizeof(address), "/ch%dpitch", (int)channel);
  sendOSCMessage(address, value);  // /ch<channel>pitch (<value>,)
}

static void onAftertouch(Channel channel, byte value) {
#if (VERBOSE > 0)
  DEBUG_SERIAL.printf("C%u: AT=%u\r\n", channel, value);
#endif
  char address[12];
  snprintf(address, sizeof(address), "/after%d", (int)channel);
  sendOSCMessage(address, value); // /after<channel> (<value>,)
}

static void onPolyphonicAftertouch(Channel channel, byte note, byte amount) {
#if (VERBOSE > 0)
  DEBUG_SERIAL.printf("C%u: PAT#%u=%u\r\n", channel, note, amount);
#endif
}

static void onSysEx(byte* array, unsigned size) {
#if (VERBOSE > 1)
  DEBUG_SERIAL.printf("SysEx:\r\n");
#endif
  unsigned multipleOf8 = size / 8;
  unsigned remOf8 = size % 8;
  for (unsigned idx = 0; idx < multipleOf8; idx++) {
#if (VERBOSE > 1)
    for (unsigned jdx = 0; jdx < 8; jdx++) {
      DEBUG_SERIAL.printf("%02x ", *array++);
    }
    DEBUG_SERIAL.printf("\r\n");
#endif
  }
#if (VERBOSE > 1)
  for (unsigned idx = 0; idx < remOf8; idx++) {
    DEBUG_SERIAL.printf("%02x ", *array++);
  }
  DEBUG_SERIAL.printf("\r\n");
#endif
}

static void onSMPTEqf(byte data) {
  uint8_t type = (data >> 4) & 0xF;
  data &= 0xF;
  static const char* fps[4] = { "24", "25", "30DF", "30ND" };
#if (VERBOSE > 1)
  switch (type) {
    case 0: DEBUG_SERIAL.printf("SMPTE FRM LS %u \r\n", data); break;
    case 1: DEBUG_SERIAL.printf("SMPTE FRM MS %u \r\n", data); break;
    case 2: DEBUG_SERIAL.printf("SMPTE SEC LS %u \r\n", data); break;
    case 3: DEBUG_SERIAL.printf("SMPTE SEC MS %u \r\n", data); break;
    case 4: DEBUG_SERIAL.printf("SMPTE MIN LS %u \r\n", data); break;
    case 5: DEBUG_SERIAL.printf("SMPTE MIN MS %u \r\n", data); break;
    case 6: DEBUG_SERIAL.printf("SMPTE HR LS %u \r\n", data); break;
    case 7: DEBUG_SERIAL.printf("SMPTE HR MS %u FPS:%s\r\n", data & 0x1, fps[(data >> 1) & 3]); break;
    default:DEBUG_SERIAL.printf("invalid SMPTE data byte %u\r\n", data); break;
  }
#endif
}

static void onSongPosition(unsigned beats) {
#if (VERBOSE > 0)
  DEBUG_SERIAL.printf("SongP=%u\r\n", beats);
#endif
}

static void onSongSelect(byte songnumber) {
#if (VERBOSE > 0)
  DEBUG_SERIAL.printf("SongS#%u\r\n", songnumber);
#endif
}

static void onTuneRequest() {
#if (VERBOSE > 0)
  DEBUG_SERIAL.printf("Tune\r\n");
#endif
}

static void onMidiClock() {
#if (VERBOSE > 1)
  DEBUG_SERIAL.printf("Clock\r\n");
#endif
}

static void onMidiStart() {
#if (VERBOSE > 0)
  DEBUG_SERIAL.printf("Start\r\n");
#endif
}

static void onMidiContinue() {
#if (VERBOSE > 0)
  DEBUG_SERIAL.printf("Cont\r\n");
#endif
}

static void onMidiStop() {
#if (VERBOSE > 0)
  DEBUG_SERIAL.printf("Stop\r\n");
#endif
}

static void onActiveSense() {
#if (VERBOSE > 1)
  DEBUG_SERIAL.printf("ASen\r\n");
#endif
}

static void onSystemReset() {
#if (VERBOSE > 0)
  DEBUG_SERIAL.printf("SysRst\r\n");
#endif
}

static void onMidiTick() {
#if (VERBOSE > 1)
  DEBUG_SERIAL.printf("Tick\r\n");
#endif
}

static void onMidiInWriteFail(uint8_t devAddr, uint8_t cable, bool fifoOverflow) {
#if (VERBOSE > 0)
  if (fifoOverflow)
    DEBUG_SERIAL.printf("Dev %u cable %u: MIDI IN FIFO overflow\r\n", devAddr, cable);
  else
    DEBUG_SERIAL.printf("Dev %u cable %u: MIDI IN FIFO error\r\n", devAddr, cable);
#endif
}

static void registerMidiInCallbacks(uint8_t devAddr, uint8_t midiInCable) {
  auto dev = usbhMIDI.getDevFromDevAddr(devAddr);
  if (dev == nullptr)
    return;  // invalid device address
  if (midiInCable >= dev->getNumInCables())
    return;  // invalid MIDI IN cable number

  auto intf = usbhMIDI.getInterfaceFromDeviceAndCable(devAddr, midiInCable);
  if (intf == nullptr)
    return;                                               // unexpected error
  intf->setHandleNoteOff(onNoteOff);                      // 0x80
  intf->setHandleNoteOn(onNoteOn);                        // 0x90
  intf->setHandleAfterTouchPoly(onPolyphonicAftertouch);  // 0xA0
  intf->setHandleControlChange(onControlChange);          // 0xB0
  intf->setHandleProgramChange(onProgramChange);          // 0xC0
  intf->setHandleAfterTouchChannel(onAftertouch);         // 0xD0
  intf->setHandlePitchBend(onPitchBend);                  // 0xE0
  intf->setHandleSystemExclusive(onSysEx);                // 0xF0, 0xF7
  intf->setHandleTimeCodeQuarterFrame(onSMPTEqf);         // 0xF1
  intf->setHandleSongPosition(onSongPosition);            // 0xF2
  intf->setHandleSongSelect(onSongSelect);                // 0xF3
  intf->setHandleTuneRequest(onTuneRequest);              // 0xF6
  intf->setHandleClock(onMidiClock);                      // 0xF8
  // 0xF9 as 10ms Tick is not MIDI 1.0 standard but implemented in the Arduino MIDI Library
  intf->setHandleTick(onMidiTick);              // 0xF9
  intf->setHandleStart(onMidiStart);            // 0xFA
  intf->setHandleContinue(onMidiContinue);      // 0xFB
  intf->setHandleStop(onMidiStop);              // 0xFC
  intf->setHandleActiveSensing(onActiveSense);  // 0xFE
  intf->setHandleSystemReset(onSystemReset);    // 0xFF
  intf->setHandleError(onMidiError);

  dev->setOnMidiInWriteFail(onMidiInWriteFail);
}

static void unregisterMidiInCallbacks(uint8_t devAddr, uint8_t midiInCable){
  auto dev = usbhMIDI.getDevFromDevAddr(devAddr);
  if (dev == nullptr)
    return;  // invalid device address
  if (midiInCable >= dev->getNumInCables())
    return;  // invalid MIDI IN cable number

  auto intf = usbhMIDI.getInterfaceFromDeviceAndCable(devAddr, midiInCable);
  if (intf == nullptr)
      return;
  intf->disconnectCallbackFromType(NoteOn);
  intf->disconnectCallbackFromType(NoteOff);
  intf->disconnectCallbackFromType(AfterTouchPoly);
  intf->disconnectCallbackFromType(ControlChange);
  intf->disconnectCallbackFromType(ProgramChange);
  intf->disconnectCallbackFromType(AfterTouchChannel);
  intf->disconnectCallbackFromType(PitchBend);
  intf->disconnectCallbackFromType(SystemExclusive);
  intf->disconnectCallbackFromType(TimeCodeQuarterFrame);
  intf->disconnectCallbackFromType(SongPosition);
  intf->disconnectCallbackFromType(SongSelect);
  intf->disconnectCallbackFromType(TuneRequest);
  intf->disconnectCallbackFromType(Clock);
  // 0xF9 as 10ms Tick is not MIDI 1.0 standard but implemented in the Arduino MIDI Library
  intf->disconnectCallbackFromType(Tick);
  intf->disconnectCallbackFromType(Start);
  intf->disconnectCallbackFromType(Continue);
  intf->disconnectCallbackFromType(Stop);
  intf->disconnectCallbackFromType(ActiveSensing);
  intf->disconnectCallbackFromType(SystemReset);
  intf->setHandleError(nullptr);
    
  dev->setOnMidiInWriteFail(nullptr);
}

/* CONNECTION MANAGEMENT */
// return the index of the stored devAddr or -1 if not connected
static int getConnectedIdx(uint8_t devAddr) {
  int idx = 0;
  for (; idx < numConnectedDevices && connectedDevAddrs[idx] != devAddr; idx++) {
  }
  return idx < numConnectedDevices ? idx : -1;
}

static void onMIDIconnect(uint8_t devAddr, uint8_t nInCables, uint8_t nOutCables) {
#if (VERBOSE > 0)
  DEBUG_SERIAL.printf("MIDI device at address %u has %u IN cables and %u OUT cables\r\n", devAddr, nInCables, nOutCables);
#endif
  int idx = getConnectedIdx(devAddr);
  if (idx == -1) {
    // this is expected
    if (numConnectedDevices >= RPPICOMIDI_TUH_MIDI_MAX_DEV) {
#if (VERBOSE > 0)
      DEBUG_SERIAL.printf("Error: %u is too many connected devices\r\n", numConnectedDevices + 1);
#endif
    }
    connectedDevAddrs[numConnectedDevices++] = devAddr;
    for (uint8_t inCable = 0; inCable < nInCables; inCable++) {
      registerMidiInCallbacks(devAddr, inCable);
    }
    // Turn on the LED to indicate that at least one device is connected
    analogWrite(Utils::LED_PIN, Utils::LED_INTENSITY);
  } else {
#if (VERBOSE > 0)
    DEBUG_SERIAL.printf("unexpected device address %u already connected at idx=%d\r\n", devAddr, idx);
#endif
  }
}

static void onMIDIdisconnect(uint8_t devAddr) {
#if (VERBOSE > 0)
    DEBUG_SERIAL.printf("MIDI device at address %u unplugged\r\n", devAddr);
#endif
  if (numConnectedDevices == 0) {
    // this is not expected
#if (VERBOSE > 0)
    DEBUG_SERIAL.printf("got disconnected event with no connected devices\r\n");
#endif
    return;
  }
  int idx = getConnectedIdx(devAddr);
  if (idx == -1) {
    // This is not expected
#if (VERBOSE > 0)
    DEBUG_SERIAL.printf("Disconnected device address %u not found\r\n", devAddr);
#endif
  } else {
    uint8_t nInCables = usbhMIDI.getNumInCables(devAddr);
    for (uint8_t inCable = 0; inCable < nInCables; inCable++) {
      unregisterMidiInCallbacks(devAddr, inCable);
    }

    // replace the disconnected device with the last one on the list
    connectedDevAddrs[idx] = connectedDevAddrs[--numConnectedDevices];
    // if the last device was disconnected, turn off the LED
    if (numConnectedDevices == 0) {
      digitalWrite(Utils::LED_PIN, LOW);
    }
  }
#if (VERBOSE > 0)
  DEBUG_SERIAL.printf("numConnectedDevices=%u\r\n", numConnectedDevices);
  for (idx = 0; idx < numConnectedDevices; idx++) {
    DEBUG_SERIAL.printf("connectedDevAddrs[%d]=%u\r\n", idx, connectedDevAddrs[idx]);
  }
#endif
}


/* MAIN LOOP FUNCTIONS */
class OscMode {
public:
  OscMode() {}

  void setup() {
      pinMode(Utils::LED_PIN, OUTPUT);
#if (VERBOSE > 0)
    while (!DEBUG_SERIAL) ;  // wait for serial port
    DEBUG_SERIAL.println("\nAttempting to connect to WiFi");
#endif

    Utils::startLittleFS();  // Start LittleFS

    String configSSID = Utils::readWiFiConfig(Utils::ConfigLine::SSID);
    String configPWD = Utils::readWiFiConfig(Utils::ConfigLine::PASSWORD);
    String configIP = Utils::readWiFiConfig(Utils::ConfigLine::IP);
    String configPort = Utils::readWiFiConfig(Utils::ConfigLine::PORT);

    configSSID.toCharArray(cconfigSSID, sizeof(cconfigSSID));
    configPWD.toCharArray(cconfigPWD, sizeof(cconfigPWD));
    configIP.toCharArray(cOutIP, sizeof(cOutIP));
    configPort.toCharArray(cOutPort, sizeof(cOutPort));

#if (VERBOSE > 0)
    DEBUG_SERIAL.println("Reading from config");
    DEBUG_SERIAL.print("SSID: \"");
    DEBUG_SERIAL.print(cconfigSSID);
    DEBUG_SERIAL.println("\"");
    DEBUG_SERIAL.print("PWD: \"");
    DEBUG_SERIAL.print(cconfigPWD);
    DEBUG_SERIAL.println("\"");
    DEBUG_SERIAL.print("IP: \"");
    DEBUG_SERIAL.print(cOutIP);
    DEBUG_SERIAL.println("\"");
    DEBUG_SERIAL.print("PORT: \"");
    DEBUG_SERIAL.print(cOutPort);
    DEBUG_SERIAL.println("\"");
#endif

    WiFi.begin(cconfigSSID, cconfigPWD);  // Config WiFi
    while (WiFi.status() != WL_CONNECTED) {
      delay(500);
#if (VERBOSE > 0)
      DEBUG_SERIAL.print(".");
#endif
    }
#if (VERBOSE > 0)
    DEBUG_SERIAL.println("\nConnected to WiFi");
#endif

    IPAddress outIp;
    outIp.fromString(cOutIP);  // Convert IP string to IPAddress object
    unsigned int outPort = atoi(cOutPort);  // Convert port string to unsigned int

    if (udp.connect(outIp, outPort)) {  // Try to send something to the server (to establish the connection)
#if (VERBOSE > 0)
      DEBUG_SERIAL.println("UDP connected");
      udp.print("/test Hello");
#endif
    }

    usbhMIDI.begin(&USBHost, 0, onMIDIconnect, onMIDIdisconnect);
  }

  void loop() {
    while (1) {
      // Update the USB Host
      USBHost.task();

      // Handle any incoming data; triggers MIDI IN callbacks
      usbhMIDI.readAll();

      // Other non-MIDI processing (Status LED, etc.)
      Utils::blinkLED(numConnectedDevices);
    }
  }
};


#endif // _OSCMODE_H