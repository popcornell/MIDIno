/*
 *  Copyright (c) 2016 Samuele Cornell  
 *
 *  Permission is hereby granted, free of charge, to any person obtaining a copy
 *  of this software and associated documentation files (the "Software"), to deal
 *  in the Software without restriction, including without limitation the rights
 *  to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
 *  copies of the Software, and to permit persons to whom the Software is
 *  furnished to do so, subject to the following conditions:
 *
 *  The above copyright notice and this permission notice shall be included in all
 *  copies or substantial portions of the Software.
 *
 *  THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 *  IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 *  FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
 *  AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 *  LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
 *  OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
 *  SOFTWARE.
 *
 */
 
 #define BAUD_RATE 31250 // standard MIDI baud rate. If one want to use this program with USB using a Serial to MIDI middleware running on Windows/OSX change accordingly
 
// i have used DFRobot MIDI Shield to connect MIDI cables to the nRF51822 board but there are other commercially available similar products, also it is possible to interface directly a MIDI cable via an optocoupler (several projects are already available online ) 
 
#define LIGHT_SHOW 1 // 0 if visual feedback isn't needed  (saves energy )
 
#define TX_LED D6 // CHANGE THIS ACCORDINGLY the state of this led will change whenever a message is to be sent over ble-midi
 
#define RX_LED D7 // CHANGE THIS ACCORDINGLY the state of this led will change whenever a message received over ble-midi
 
 
/*******************************************************************************************************
PERFORMANCE TWEAKS
*******************************************************************************************************/
 
 
#define BUFSERIAL_LENGHT 256 // define the software circular buffer lenght used for the UART. // work in progress for Arduino IDE 
 
 
#define ONLY_MIDI_to_BLEMIDI 0 // if only unidirectional MIDI to BLE-MIDI is desired set this to 1  
 
 
#define ONLY_BLEMIDI_to_MIDI 0 // if only unidirectional BLE-MIDI to MIDI is desired set this to 1 
 
// unidirectional operation allows to save energy. It also leads to better performance as if ONLY MIDI to BLE MIDI is required, for example it is  possible to shorten the SENDBLE_INTERVAL
// without reliability issues (to a certain extent).   
 
#define SENDBLEMIDI_INTERVAL 0.01 // this defines how frequently MIDI Events from the UART are polled, parsed and then sent via BLE-MIDI. 
                                 // a lower value means less latency but it also increase energy comsuption and if is set too low can cause reliability issues in MIDI to BLE-MIDI operation (especially for long SysEx messages).  
 
 
/***************************************************************************************************************
CONNECTION PARAMETERS 
***************************************************************************************************************/
 
namespace Config { 
 
// 
const int minConnectionInterval = 6; // (1.25 ms units)  
const int maxConnectionInterval = 15; // (1.25 ms units)
const int slaveLatency          = 0;
const int supervisionTimeout    = 500; // (10 ms units)
 
 
 
} 
