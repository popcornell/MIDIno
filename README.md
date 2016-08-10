# MIDIno
MIDI (through standard 5-DIN MIDI cables) to BLE-MIDI bridging with Arduino-compatible boards project.    

This project was inspired by Matt Sieren (sieren) blidino Arduino USB-MIDI to BLE-MIDI project https://github.com/sieren/blidino.

The purpose of this project is to allow MIDI devices that doesn't support USB-MIDI to communicate wires-free through the Midi Manufacturers Association MIDI over Bluetooth Low Energy (BLE-MIDI) protocol.   

# Supported Boards

[RedBearLab nRF51822](https://github.com/popcornell/MIDIno/tree/master/MIDI_to_BLE-MIDI_bridge%20nRF51822)

Currently the only board supported is the redbearlab nRF51822 Arduino-compatible Board. 

This repo contains the code for the Arduino IDE but as this same board supports [mbed OS](https://www.mbed.com/en/development/mbed-os/) ( see [redbearslab nRF51822 description](http://redbearlab.com/redbearlab-nrf51822/) ) the program was originally developed using that platform.    

Also currently this Arduino IDE port here lacks of additional uart software buffering. 

As such i strongly recommend, for now , to use instead the mbed OS version available here [MIDI-to-BLE-MIDI-bridge (mbed OS)](https://developer.mbed.org/users/popcornell/code/MIDI-to-BLE-MIDI-bridge/). This was more thoroughly tested and should be considered more reliable.  





