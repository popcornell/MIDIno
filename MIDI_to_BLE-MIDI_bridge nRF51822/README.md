
#RedBearLab nRF51822 MIDI to BLE MIDI Bridge

![alt tag](https://developer.mbed.org/media/uploads/popcornell/wp_20160713_22_30_15_rich.jpg)


To interface this board with MIDI cables i have used a DFRobot MIDI Shield. 
However any other Arduino-compatible shield can be used or one can directly connect the UART pins to the MIDI cables using an optocoupler. 

#Requirements 

The program here is intended to be compiled with the Arduino IDE.
Please refer to [redbearlab nRF51822 user guide](http://redbearlab.com/getting-started-nrf51822/) to how to set up this board for Arduino IDE. 

Alternatively the original and **recommended** program for mbed OS and mbed Compiler is available here: https://developer.mbed.org/users/popcornell/code/MIDI-to-BLE-MIDI-bridge/


# Current Issues 

Arduino version lacks software buffering for uart
