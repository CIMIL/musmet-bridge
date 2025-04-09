# Musical Metaverse _pico_-Bridge

Hardware and software solution that let's you use your MIDI devices in a _**Musical Metaverse**_. Connect any class-compliant MIDI device and map its MIDI input into OSC messages. 
These OSC messages can then be sent to a VR/XR musical ecosystem (e.g., PatchWorld) or other compatible platforms.

## Hardware
The **_pico-bridge_**'s hardware consists of the following components:

- **Raspberry Pi Pico W**: Serves both as the MIDI host and as the OSC client, handling communication and processing.
- **USB-to-microUSB adapter**: USB-A cables are the most common.
- **Breakout Board**: Provides a stable 5V power supply to the Pico and connected MIDI devices.

## Features

- **MIDI Host Functionality**: Connect any class-compliant MIDI device to the _pico-Bridge_.
- **OSC Message Mapping**: Translate MIDI input into OSC messages for seamless integration with PatchWorld or any other musical metaverse ecosystem.
- **Customizable Mapping**: Modify the MIDI-to-OSC mapping to suit your specific needs.

## Debugging
To debug the project and view serial messages, you will need a DebugProbe (you can also use a Pico as a PicoProbe).


## MIDI2OSC Mapping

- ✅ **Note On/Off**: Trigger and release notes.
- ✅ **Pitch Bending**: Adjust pitch dynamically.
- ✅ **Control Changes (CC)**: Modify parameters such as volume, pan, or modulation.
- ✅ **Aftertouch**: Respond to pressure applied to keys after they are pressed.

- ❌ **Program Change**: Switch instrument or patch presets.
- ❌ **System Exclusive (SysEx)**: Manufacturer-specific messages for advanced device control.
- ❌ **MIDI Time Code (MTC)**: Synchronize playback with other devices.
- ❌ **Polyphonic Aftertouch**: Individual pressure sensitivity for each note.
- ❌ **Song Select**: Choose a specific song in a sequence.
- ❌ **Song Position Pointer**: Indicate a specific position in a song for playback.
- ❌ **System Real-Time Messages**: Clock, start, stop, and continue for synchronization.
