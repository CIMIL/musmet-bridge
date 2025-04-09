# Musical Metaverse _pico_-Bridge

Hardware and software solution that let's you use your MIDI devices in a _**Musical Metaverse**_. Connect any class-compliant MIDI device and map its MIDI input into OSC messages. 
These OSC messages can then be sent to a VR/XR musical ecosystem (e.g., PatchWorld) or other compatible platforms.

## Hardware
The hardware consists of the following components:

- **Raspberry Pi Pico W**: Serves both as the MIDI host and as the OSC client, handling communication and processing.
- **Breakout Board**: Provides a stable power supply to the Raspberry Pi Pico and connected peripherals.

## Features

- **MIDI Host Functionality**: Connect any class-compliant MIDI device to the Raspberry Pi Pico W.
- **OSC Message Mapping**: Translate MIDI input into OSC messages for seamless integration with PatchWorld.
- **Customizable Mapping**: Modify the MIDI-to-OSC mapping to suit your specific needs.

## Debugging
To debug the project and view serial messages, you will need a Debug Probe (you can use a RPi Pico).
