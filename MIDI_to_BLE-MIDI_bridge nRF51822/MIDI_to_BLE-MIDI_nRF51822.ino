
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
 
 /*
 *   Thanks to Matthias Frick and his project blidino https://github.com/sieren/blidino 
 *   which also uses RedBearslab nRF51822 and implements a USB-MIDI to BLE-MIDI bridge. 
 *   His work made this possible and served as the main inspiration for this project . 
 *   The content of BLEMIDI_MIDI_Parser.h file is for the most part taken from his blidino project.  
 */

#include <Arduino.h>

#include <BLE_API.h>

#include <nordic_common.h>
 
//#include "BufferedSerial.h"
 
#include "config.h"
 
 
 
#if ONLY_MIDI_to_BLEMIDI==0 
 
#include "BLEMIDI_MIDI_Parser.h" 
 
#endif
 
#if ONLY_BLEMIDI_to_MIDI==0   
 
#include "MIDI_BLEMIDI_Parser.h"
 
#endif
 
 
 
#define TXRX_BUF_LEN                    20 
 
#define RX_BUF_LEN                      20 
 
/******************************************************************************************************************************
*INITIALIZE VARIABLES, BUFFERS and BLE-MIDI Service and Characteristic
******************************************************************************************************************************/
 
Ticker sendBLEMIDI_Ticker ; 

 
Timer t; // timer used for BLE-MIDI timestamps 
 
bool isConnected; 

#if LIGHT_SHOW==1  
bool tx_led_state , rx_led_state ; 
#endif 

////////////////////////////////////////////
bool sendBLEMIDI_flag = false ; 
 
////////////
 
 
BLEDevice ble; // BLE_API  
 
static Gap::ConnectionParams_t connectionParams;
 
 
// MIDI BLE Service and Characteristic UUID ( see Apple BLE-MIDI Spec. and Midi manufacter Association BLE-MIDI Spec.) 
 
static const uint8_t service_uuid[] = {0x03, 0xB8, 0x0E, 0x5A, 0xED, 0xE8, 0x4B, 0x33, 0xA7, 0x51, 0x6C, 0xE3, 0x4E, 0xC4, 0xC7, 0};
static const uint8_t characteristic_uuid[]   = {0x77, 0x72, 0xE5, 0xDB, 0x38, 0x68, 0x41, 0x12, 0xA1, 0xA9, 0xF2, 0x66, 0x9D, 0x10, 0x6B, 0xF3};
 
static const uint8_t service_uuid_rev[] = {0, 0xC7, 0xC4, 0x4E, 0xE3, 0x6C, 0x51, 0xA7, 0x33, 0x4B, 0xE8, 0xED, 0x5A, 0x0E, 0xB8, 0x03};
 
uint8_t txPayload[TXRX_BUF_LEN] = {0,};
 
 
GattCharacteristic  midiCharacteristic(characteristic_uuid, txPayload, 0, 20,GattCharacteristic::BLE_GATT_CHAR_PROPERTIES_WRITE |
                                                   GattCharacteristic::BLE_GATT_CHAR_PROPERTIES_WRITE_WITHOUT_RESPONSE |
                                                                   GattCharacteristic::BLE_GATT_CHAR_PROPERTIES_NOTIFY |
                                                                     GattCharacteristic::BLE_GATT_CHAR_PROPERTIES_READ);
 
GattCharacteristic *midiChars[] = {&midiCharacteristic};
GattService BLEMIDIService(service_uuid, midiChars,sizeof(midiChars) / sizeof(GattCharacteristic *));



/*********************************************************************************************************************
MIDI to BLE-MIDI  
*********************************************************************************************************************/
 
                                
#if ONLY_BLEMIDI_to_MIDI==0     
               
 
void sendBLEMIDI_Callback (void)
{
 
    sendBLEMIDI_flag = true ; 
    
    /**** This callback is called within an ISR with frequency set by the sendBLEMIDI_Ticker, because it is called in an Interrupt context it is preferably 
    doing the MIDI to BLEMIDI task in the main, the callback only set a flag which would be checked in the main loop. 
    This is because all the BLEMIDI to MIDI conversion is already performed within Interrupt context. ( thus as a consequence BLEMIDI to MIDI has higher priority )  
    *****/
}
 

void sendBLEMIDI(void) 
{
 
 
// MIDI::MIDI_to_BLEMIDI_Parser() parses incoming MIDI events from the UART (see MIDI_BLEMIDI_Parser.h) 
    while(isConnected == true && MIDI::MIDI_to_BLEMIDI_Parser()==true) {
 
        // a valid MIDI message has been parsed from the UART buffer
 
#if LIGHT_SHOW==1
    tx_led_state = !(tx_led_state) ; 
    digitalWrite(TX_LED, tx_led_state) ; 
#endif
 
        uint8_t BLEmidi_out[SysExMaxSize] ;
        uint8_t SysEx_Array_lenght ;
 
        uint16_t ticks = t.read_ms() & 0x1fff; // read timer for timestamps 
 
        if(MIDI::mMessage.sysexArray[0] == MIDI::SystemExclusive)   { // message is  SysEx
 
 
            SysEx_Array_lenght = MIDI::mMessage.getSysExSize() ; // get SysEx message lenght
 
            uint8_t position = 0; // position for SysexArray 
 
            // header
           
            BLEmidi_out[position++] = 0x80 | ((ticks >> 7) & 0x3f); // header & timestampHigh
            BLEmidi_out[position++] = 0x80 | (ticks & 0x7f); // timestampLow
 
            for (int i = 0; i < SysEx_Array_lenght; i++) {
                if (i == SysEx_Array_lenght - 1) {
                    // modify last byte
                    BLEmidi_out[position++] = 0x80 | (ticks & 0x7f);
 
                    if (position == 20) {
                        
                        
                        ble.updateCharacteristicValue(midiCharacteristic.getValueAttribute().getHandle(), BLEmidi_out, 20);
 
                        position = 0;
                        // header
                        BLEmidi_out[position++] = 0x80 | (ticks >> 7) & 0x3f;
                    }
                }
                BLEmidi_out[position++] = MIDI::mMessage.sysexArray[i];
                if (position == 20) {
                  ble.updateCharacteristicValue(midiCharacteristic.getValueAttribute().getHandle(), BLEmidi_out, 20);
                
                    position = 0;
                    // header
                    BLEmidi_out[position++] = 0x80 | (ticks >> 7) & 0x3f;
                }
 
                ticks = t.read_ms() & 0x1fff;
            }
 
            if (position > 0) {
                // send remains
             ble.updateCharacteristicValue(midiCharacteristic.getValueAttribute().getHandle(), BLEmidi_out, position);
             
            }
            
           
            MIDI::mMessage.sysexArray[0]= 0 ; // reset
 
        } 
 
 
        // message is not SysEx
        
        
        else {
            
            
            if(MIDI::mMessage.data1 == 0 ) { // no data1 only status 
            
            BLEmidi_out[0] = 0x80 | ((ticks >> 7) & 0x3f);
            BLEmidi_out[1] = 0x80 | (ticks & 0x7f);
            BLEmidi_out[2] = MIDI::mMessage.channel+MIDI::mMessage.type;
        
             ble.updateCharacteristicValue(midiCharacteristic.getValueAttribute().getHandle(), BLEmidi_out , 3);
            
            }
            
            if(MIDI::mMessage.data2 == 0 ) { // no data2 
            
            BLEmidi_out[0] = 0x80 | ((ticks >> 7) & 0x3f);
            BLEmidi_out[1] = 0x80 | (ticks & 0x7f);
            BLEmidi_out[2] = MIDI::mMessage.channel+MIDI::mMessage.type;
            BLEmidi_out[3] = MIDI::mMessage.data1 ;
 
            ble.updateCharacteristicValue(midiCharacteristic.getValueAttribute().getHandle(), BLEmidi_out , 4);
            
            } 
            
            if(MIDI::mMessage.data2 != 0 ) { 
            
            BLEmidi_out[0] = 0x80 | ((ticks >> 7) & 0x3f);
            BLEmidi_out[1] = 0x80 | (ticks & 0x7f);
            BLEmidi_out[2] = MIDI::mMessage.channel+MIDI::mMessage.type;
            BLEmidi_out[3] = MIDI::mMessage.data1 ;
            BLEmidi_out[4] = MIDI::mMessage.data2 ;
 
            ble.updateCharacteristicValue(midiCharacteristic.getValueAttribute().getHandle(), BLEmidi_out , 5);
            
            } 
 
        }// end else
 
    }// outer if
 
// invalid message or not connected
 
}
 
#endif 

/******************************************************************************************************************************
* BLE CALLBACKS 
******************************************************************************************************************************/
 
                                
void disconnectionCallback(Gap::Handle_t handle,
  Gap::DisconnectionReason_t reason) 
{
    //device disconnected 
    
    isConnected = false ; 
    
#if ONLY_BLEMIDI_to_MIDI==0   
   sendBLEMIDI_Ticker.detach() ; // stop Ticker to save energy 
#endif
    
    ble.startAdvertising(); // start advertising
}
 
 
void connectionCallback(const Gap::ConnectionCallbackParams_t* params) {
 
 
isConnected=true ; 
 
// try update conn.parameters 
 
connectionParams.minConnectionInterval        = Config::minConnectionInterval;
connectionParams.maxConnectionInterval        = Config::maxConnectionInterval;
connectionParams.slaveLatency                 = Config::slaveLatency;
connectionParams.connectionSupervisionTimeout = Config::supervisionTimeout;
 
ble.updateConnectionParams(params->handle ,&connectionParams);
 
 
//start timers here
#if ONLY_BLEMIDI_to_MIDI==0    
sendBLEMIDI_Ticker.attach( sendBLEMIDI_Callback, SENDBLEMIDI_INTERVAL); // every SENDBLEMIDI_INTERVAL seconds calls sendBLEMIDI_Callback (ISR)   
 
t.start();  // start the timer used for BLEMIDI timestamps  
 
#endif 
 
} 
 
 
/****************************************************************************************************************************
BLE-MIDI to MIDI 
****************************************************************************************************************************/
 
 
#if ONLY_MIDI_to_BLEMIDI==0 
 
void parseIncoming(uint8_t *buffer, uint16_t bytesRead) {  // parse BLE-MIDI Events that have been written on the MIDI Characteristic 
  for (int i = 1; i < bytesRead; i++)
  {
    parseMidiEvent(buffer[0], buffer[i]); // parse and send through UART the MIDI Events received through BLE (see BLE_MIDI_Parser.h)
  }
}
                                
 
 
void onDataWritten(const GattWriteCallbackParams *Handler) // this functions is called within an ISR every time data has been written on nRF51822 GATT Server MIDI Characteristic   
{

#if LIGHT_SHOW==1
        rx_led_state = !(rx_led_state) ; 
        digitalWrite(RX_LED, rx_led_state);
        
#endif

 
    uint8_t buf[TXRX_BUF_LEN];
    uint16_t bytesRead;
    if (Handler->handle == midiCharacteristic.getValueAttribute().getHandle()) {
        ble.readCharacteristicValue(midiCharacteristic.getValueAttribute().getHandle(),
                                    buf, &bytesRead);
        parseIncoming(buf, bytesRead);
 
    }
 
}
  
#endif  



/**********************************************************************************************************  
 *   SETUP 
 *   
 *********************************************************************************************************/

void setup() {

#if LIGHT_SHOW==1
pinMode (RX_LED,OUTPUT);  // sets the two LEDS on the MIDI Shield as outputs, these will be used as a visual feedback for debug 
pinMode (TX_LED,OUTPUT); //
#endif 

#if LIGHT_SHOW==1
    digitalWrite(RX_LED, HIGH);
    rx_led_state = false ; 
    digitalWrite(TX_LED, HIGH);
    tx_led_state =false; 
#endif
 
 
 
    Serial.begin(BAUD_RATE) ; // set UART baud rate to 31250 (MIDI standard)
 
    ble.init();
    ble.onDisconnection(disconnectionCallback);
    ble.onConnection(connectionCallback) ;
 
#if ONLY_MIDI_to_BLEMIDI==0
    ble.onDataWritten(onDataWritten);
#endif
    
    //conn. parameters ( rejected on iOS/OSX  see Apple BLE peripheral design guidelines but can work on Android ) 
    
    connectionParams.minConnectionInterval        = Config::minConnectionInterval;
    connectionParams.maxConnectionInterval        = Config::maxConnectionInterval;
    connectionParams.slaveLatency                 = Config::slaveLatency;
    connectionParams.connectionSupervisionTimeout = Config::supervisionTimeout;
    ble.setPreferredConnectionParams(&connectionParams);
    ble.getPreferredConnectionParams(&connectionParams);
    
    
    /* setup advertising */
    ble.accumulateAdvertisingPayload(GapAdvertisingData::BREDR_NOT_SUPPORTED);
    ble.accumulateAdvertisingPayload(GapAdvertisingData::SHORTENED_LOCAL_NAME,
                                     (const uint8_t *)"nRF51", sizeof("nRF51") - 1);
    ble.accumulateAdvertisingPayload(GapAdvertisingData::COMPLETE_LIST_128BIT_SERVICE_IDS,
                                     (const uint8_t *)service_uuid_rev, sizeof(service_uuid_rev));
 
    ble.accumulateScanResponse(GapAdvertisingData::SHORTENED_LOCAL_NAME,
                               (const uint8_t *)"nRF51", sizeof("nRF51") - 1);
    ble.accumulateScanResponse(GapAdvertisingData::COMPLETE_LIST_128BIT_SERVICE_IDS,(const uint8_t *)service_uuid_rev, sizeof(service_uuid_rev));
    ble.setAdvertisingType(GapAdvertisingParams::ADV_CONNECTABLE_UNDIRECTED);
 
    /* 100ms; in multiples of 0.625ms. */
    ble.setAdvertisingInterval(160);
 
    // set adv_timeout, in seconds
    ble.setAdvertisingTimeout(0);
    ble.addService(BLEMIDIService);
 
    //Set Device Name
    ble.setDeviceName((const uint8_t *)"nRF51");
 
    ble.startAdvertising();
  

}

void loop() {
  
 #if ONLY_BLEMIDI_to_MIDI==0    
  
            if(sendBLEMIDI_flag == true ) { // check if the flag is set 
 
            sendBLEMIDI_flag=false ;
            sendBLEMIDI() ; // parse MIDI Events from UART and send them over BLE
 
        }
#endif
 
        ble.waitForEvent(); // sleep 
 

}
