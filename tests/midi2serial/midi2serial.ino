/**
 * This demo program is designed to test the USB MIDI Host driver for up to 4
 * USB MIDI devices connected to a USB Hub and then to the USB host port. It
 * sends to each virtual MIDI OUT cable of each connected USB MIDI device on the hub
 * the sequence of half-steps from B-flat to D whose note numbers correspond to the
 * transport button LEDs on a Mackie Control compatible control surface. It also
 * prints to a UART serial port console the messages received from the USB MIDI device.
 *
 * A frustration of the Arduino MIDI Library is that the MIDI IN callback functions
 * use C-style function pointers and do not have user data associated with them. It
 * is therefore not possible to identify the device and cable number that received MIDI
 * data without writing a unique function for each device and virtual
 * MIDI IN cable combination. For an application-specific program that relies on known
 * devices it is not too bad. A general purpose program like this would need
 * 16 x RPPICOMIDI_TUH_MIDI_MAX_DEV different functions for each MIDI IN callback.
 * This example does not do that because it would make what is supposed to be a simple
 * example too complex. Also, many applications may not require the ability to differentiate
 * where MIDI data is coming from.
 */

#if !defined(USE_TINYUSB_HOST) || !defined(USE_TINYUSB)
#error "Please use the Menu to select Tools->USB Stack: Adafruit TinyUSB Host"
#else
#warning "All Serial Monitor Output is on Serial1"
#endif

#include "EZ_USB_MIDI_HOST.h"
// USB Host object
Adafruit_USBH_Host USBHost;
USING_NAMESPACE_MIDI
USING_NAMESPACE_EZ_USB_MIDI_HOST

RPPICOMIDI_EZ_USB_MIDI_HOST_INSTANCE(usbhMIDI, MidiHostSettingsDefault)

static uint8_t connectedDevAddrs[RPPICOMIDI_TUH_MIDI_MAX_DEV];
static uint8_t numConnectedDevices = 0;

// return the index of the stored devAddr or -1 if not connected
static int getConnectedIdx(uint8_t devAddr)
{
    int idx = 0;
    for (; idx < numConnectedDevices && connectedDevAddrs[idx] != devAddr; idx++) {
    }
    return idx < numConnectedDevices ? idx:-1;
}

/* MIDI IN MESSAGE REPORTING */
static void onMidiError(int8_t errCode)
{
    Serial1.printf("MIDI Errors: %s %s %s\r\n", (errCode & (1UL << ErrorParse)) ? "Parse":"",
        (errCode & (1UL << ErrorActiveSensingTimeout)) ? "Active Sensing Timeout" : "",
        (errCode & (1UL << WarningSplitSysEx)) ? "Split SysEx":"");
}

static void onNoteOff(Channel channel, byte note, byte velocity)
{
    Serial1.printf("C%u: Note off#%u v=%u\r\n", channel, note, velocity);
}

static void onNoteOn(Channel channel, byte note, byte velocity)
{
    Serial1.printf("C%u: Note on#%u v=%u\r\n", channel, note, velocity);
}

static void onPolyphonicAftertouch(Channel channel, byte note, byte amount)
{
    Serial1.printf("C%u: PAT#%u=%u\r\n", channel, note, amount);
}

static void onControlChange(Channel channel, byte controller, byte value)
{
    Serial1.printf("C%u: CC#%u=%u\r\n", channel, controller, value);
}

static void onProgramChange(Channel channel, byte program)
{
    Serial1.printf("C%u: Prog=%u\r\n", channel, program);
}

static void onAftertouch(Channel channel, byte value)
{
    Serial1.printf("C%u: AT=%u\r\n", channel, value);
}

static void onPitchBend(Channel channel, int value)
{
    Serial1.printf("C%u: PB=%d\r\n", channel, value);
}

static void onSysEx(byte * array, unsigned size)
{
    Serial1.printf("SysEx:\r\n");
    unsigned multipleOf8 = size/8;
    unsigned remOf8 = size % 8;
    for (unsigned idx=0; idx < multipleOf8; idx++) {
        for (unsigned jdx = 0; jdx < 8; jdx++) {
            Serial1.printf("%02x ", *array++);
        }
        Serial1.printf("\r\n");
    }
    for (unsigned idx = 0; idx < remOf8; idx++) {
        Serial1.printf("%02x ", *array++);
    }
    Serial1.printf("\r\n");
}

static void onSMPTEqf(byte data)
{
    uint8_t type = (data >> 4) & 0xF;
    data &= 0xF;    
    static const char* fps[4] = {"24", "25", "30DF", "30ND"};
    switch (type) {
        case 0: Serial1.printf("SMPTE FRM LS %u \r\n", data); break;
        case 1: Serial1.printf("SMPTE FRM MS %u \r\n", data); break;
        case 2: Serial1.printf("SMPTE SEC LS %u \r\n", data); break;
        case 3: Serial1.printf("SMPTE SEC MS %u \r\n", data); break;
        case 4: Serial1.printf("SMPTE MIN LS %u \r\n", data); break;
        case 5: Serial1.printf("SMPTE MIN MS %u \r\n", data); break;
        case 6: Serial1.printf("SMPTE HR LS %u \r\n", data); break;
        case 7:
            Serial1.printf("SMPTE HR MS %u FPS:%s\r\n", data & 0x1, fps[(data >> 1) & 3]);
            break;
        default:
          Serial1.printf("invalid SMPTE data byte %u\r\n", data);
          break;
    }
}

static void onSongPosition(unsigned beats)
{
    Serial1.printf("SongP=%u\r\n", beats);
}

static void onSongSelect(byte songnumber)
{
    Serial1.printf("SongS#%u\r\n", songnumber);
}

static void onTuneRequest()
{
    Serial1.printf("Tune\r\n");
}

static void onMidiClock()
{
    Serial1.printf("Clock\r\n");
}

static void onMidiStart()
{
    Serial1.printf("Start\r\n");
}

static void onMidiContinue()
{
    Serial1.printf("Cont\r\n");
}

static void onMidiStop()
{
    Serial1.printf("Stop\r\n");
}

static void onActiveSense()
{
    Serial1.printf("ASen\r\n");
}

static void onSystemReset()
{
    Serial1.printf("SysRst\r\n");
}

static void onMidiTick()
{
    Serial1.printf("Tick\r\n");
}

static void onMidiInWriteFail(uint8_t devAddr, uint8_t cable, bool fifoOverflow)
{
    if (fifoOverflow)
        Serial1.printf("Dev %u cable %u: MIDI IN FIFO overflow\r\n", devAddr, cable);
    else
        Serial1.printf("Dev %u cable %u: MIDI IN FIFO error\r\n", devAddr, cable);
}

static void registerMidiInCallbacks(uint8_t devAddr, uint8_t midiInCable)
{
    auto dev = usbhMIDI.getDevFromDevAddr(devAddr);
    if (dev == nullptr)
        return; // invalid device address
    if (midiInCable >= dev->getNumInCables())
        return; // invalid MIDI IN cable number

    auto intf = usbhMIDI.getInterfaceFromDeviceAndCable(devAddr, midiInCable);
    if (intf == nullptr)
        return; // unexpected error
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
    intf->setHandleTick(onMidiTick);                        // 0xF9
    intf->setHandleStart(onMidiStart);                      // 0xFA
    intf->setHandleContinue(onMidiContinue);                // 0xFB
    intf->setHandleStop(onMidiStop);                        // 0xFC
    intf->setHandleActiveSensing(onActiveSense);            // 0xFE
    intf->setHandleSystemReset(onSystemReset);              // 0xFF
    intf->setHandleError(onMidiError);

    dev->setOnMidiInWriteFail(onMidiInWriteFail);
}

/* CONNECTION MANAGEMENT */
static void onMIDIconnect(uint8_t devAddr, uint8_t nInCables, uint8_t nOutCables)
{
    Serial1.printf("MIDI device at address %u has %u IN cables and %u OUT cables\r\n", devAddr, nInCables, nOutCables);
    int idx = getConnectedIdx(devAddr);
    if (idx == -1) {
        // this is expected
        if (numConnectedDevices >= RPPICOMIDI_TUH_MIDI_MAX_DEV) {
            Serial1.printf("Error: %u is too many connected devices\r\n", numConnectedDevices+1);
        }
        connectedDevAddrs[numConnectedDevices++] = devAddr;
        for (uint8_t inCable = 0; inCable < nInCables; inCable++) {
            registerMidiInCallbacks(devAddr, inCable);
        }
        // registerMidiInCallbacks(devAddr, 1);
    }
    else {
        Serial1.printf("unexpected device address %u already connected at idx=%d\r\n", devAddr, idx);
    }
}

static void onMIDIdisconnect(uint8_t devAddr)
{
    Serial1.printf("MIDI device at address %u unplugged\r\n", devAddr);
    if (numConnectedDevices == 0) {
        // this is not expected
        Serial1.printf("got disconnected event with no connected devices\r\n");
        return;
    }
    int idx = getConnectedIdx(devAddr);
    if (idx == -1) {
        // This is not expected
        Serial1.printf("Disconnected device address %u not found\r\n", devAddr);
    }
    else {
        // replace the disconnected device with the last one on the list
        connectedDevAddrs[idx] = connectedDevAddrs[--numConnectedDevices];
    }
    Serial1.printf("numConnectedDevices=%u\r\n", numConnectedDevices);
    for (idx = 0; idx < numConnectedDevices; idx++) {
        Serial1.printf("connectedDevAddrs[%d]=%u\r\n", idx, connectedDevAddrs[idx]);
    }
}

/* MAIN LOOP FUNCTIONS */

static void blinkLED(void)
{
    const uint32_t intervalMs = 1000;
    static uint32_t startMs = 0;

    static bool ledState = false;
    if ( millis() - startMs < intervalMs)
        return;
    startMs += intervalMs;

    ledState = !ledState;
    digitalWrite(LED_BUILTIN, ledState ? HIGH:LOW); 
}

static void sendNextNote()
{
    static uint8_t firstNote = 0x5b; // Mackie Control rewind
    static uint8_t lastNote = 0x5f; // Mackie Control stop
    static uint8_t offNote = lastNote;
    static uint8_t onNote = firstNote;
    // toggle NOTE On, Note Off for the Mackie Control channels 1-8 REC LED
    const uint32_t intervalMs = 1000;
    static uint32_t startMs = 0;
    if ( millis() - startMs < intervalMs)
        return; // not enough time
    startMs += intervalMs;
    for (uint8_t idx = 0; idx < numConnectedDevices; idx++) {
        uint8_t midiDevAddr = connectedDevAddrs[idx];
        auto dev = usbhMIDI.getDevFromDevAddr(midiDevAddr);
        if (dev == nullptr) {
            Serial1.printf("address %u is nullptr\r\n", midiDevAddr);
            return; // connection status has changed somehow
        }
        for (uint8_t outCable = 0; outCable < dev->getNumOutCables(); outCable++) {
            auto intf = usbhMIDI.getInterfaceFromDeviceAndCable(midiDevAddr, outCable);
            if (intf == nullptr)
                return; // not connected
            intf->sendNoteOn(offNote, 0, 1);
            intf->sendNoteOn(onNote, 0x7f, 1);            
        }
    }
    if (++offNote > lastNote)
        offNote = firstNote;
    if (++onNote > lastNote)
        onNote = firstNote;            
}

/* APPLICATION STARTS HERE */
void setup()
{
  Serial1.begin(115200);

  while(!Serial1);   // wait for serial port
  pinMode(LED_BUILTIN, OUTPUT);
  usbhMIDI.begin(&USBHost, 0, onMIDIconnect, onMIDIdisconnect);
  Serial1.println("TinyUSB MIDI Host Example");
}

void loop() {    
  while (1) {
    // Update the USB Host
    USBHost.task();

    // Handle any incoming data; triggers MIDI IN callbacks
    usbhMIDI.readAll();
    
    // Do other processing that might generate pending MIDI OUT data
    sendNextNote();
    
    // Tell the USB Host to send as much pending MIDI OUT data as possible
    usbhMIDI.writeFlushAll();
    
    // Do other non-USB host processing
    blinkLED();
  }
}

