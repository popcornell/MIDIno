/*
 *  Original work Copyright (c) 2015 Francois Best  
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
*  Modified work by Samuele Cornell.
*  The following code is adapted from Francois Best's Arduino MIDI Library v4.2 
*  (see https://github.com/FortySevenEffects/arduino_midi_library)  
*/
 
 
 
namespace MIDI {  
 
#define SysExMaxSize 128 
#define Use1ByteParsing 0
 
//typedef uint8_t byte ; // byte isn't defined in mbed OS
 
typedef uint8_t StatusByte;
typedef uint8_t DataByte;
typedef uint8_t Channel;
 
 
 
/*! Enumeration of MIDI types */
enum MidiType
{
    InvalidType           = 0x00,    ///< For notifying errors
    NoteOff               = 0x80,    ///< Note Off
    NoteOn                = 0x90,    ///< Note On
    AfterTouchPoly        = 0xA0,    ///< Polyphonic AfterTouch
    ControlChange         = 0xB0,    ///< Control Change / Channel Mode
    ProgramChange         = 0xC0,    ///< Program Change
    AfterTouchChannel     = 0xD0,    ///< Channel (monophonic) AfterTouch
    PitchBend             = 0xE0,    ///< Pitch Bend
    SystemExclusive       = 0xF0,    ///< System Exclusive
    TimeCodeQuarterFrame  = 0xF1,    ///< System Common - MIDI Time Code Quarter Frame
    SongPosition          = 0xF2,    ///< System Common - Song Position Pointer
    SongSelect            = 0xF3,    ///< System Common - Song Select
    TuneRequest           = 0xF6,    ///< System Common - Tune Request
    Clock                 = 0xF8,    ///< System Real Time - Timing Clock
    Start                 = 0xFA,    ///< System Real Time - Start
    Continue              = 0xFB,    ///< System Real Time - Continue
    Stop                  = 0xFC,    ///< System Real Time - Stop
    ActiveSensing         = 0xFE,    ///< System Real Time - Active Sensing
    SystemReset           = 0xFF,    ///< System Real Time - System Reset
};
 
 
struct MidiMessage 
{
    /*! The maximum size for the System Exclusive array.
    */
    static const unsigned sSysExMaxSize = SysExMaxSize;   // this 
 
    /*! The MIDI channel on which the message was recieved.
     \n Value goes from 1 to 16.
     */
    Channel channel;
 
    /*! The type of the message
     (see the MidiType enum for types reference)
     */
    MidiType type;
 
    /*! The first data byte.
     \n Value goes from 0 to 127.
     */
    DataByte data1;
 
    /*! The second data byte.
     If the message is only 2 bytes long, this one is null.
     \n Value goes from 0 to 127.
     */
    DataByte data2;
 
    /*! System Exclusive dedicated byte array.
     \n Array length is stocked on 16 bits,
     in data1 (LSB) and data2 (MSB)
     */
    DataByte sysexArray[sSysExMaxSize];
 
    /*! This boolean indicates if the message is valid or not.
     There is no channel consideration here,
     validity means the message respects the MIDI norm.
     */
    bool valid;
 
    inline unsigned getSysExSize() const
    {
        const unsigned size = unsigned(data2) << 8 | data1;
        return size > sSysExMaxSize ? sSysExMaxSize : size;
    } 
 
    MidiMessage() {  // initialize 
    
    channel=0;
    type= InvalidType ;
    data1= 0 ; 
    data2= 0 ; 
    valid =false ; 
    } 
    
    
}; 
   
 
    StatusByte  mRunningStatus_RX;
    StatusByte  mRunningStatus_TX;
    Channel     mInputChannel= 1 ; // default 1
    uint8_t     mPendingMessage[3];
    uint8_t     mPendingMessageExpectedLenght = 0;
    uint8_t     mPendingMessageIndex= 0;
    
    
    
    MidiMessage mMessage;
    
   
 
inline void resetInput()
{
    mPendingMessageIndex = 0;
    mPendingMessageExpectedLenght = 0;
    mRunningStatus_RX = InvalidType;
}
 
MidiType getTypeFromStatusByte(uint8_t inStatus)
{
    if ((inStatus  < 0x80) ||
        (inStatus == 0xf4) ||
        (inStatus == 0xf5) ||
        (inStatus == 0xf9) ||
        (inStatus == 0xfD))
    {
        // Data bytes and undefined.
        return InvalidType;
    }
    if (inStatus < 0xf0)
    {
        // Channel message, remove channel nibble.
        return MidiType(inStatus & 0xf0);
    }
 
    return MidiType(inStatus);
}
 
/*! \brief Returns channel in the range 1-16
 */
  
  inline Channel getChannelFromStatusByte(uint8_t inStatus)
  {
    return (inStatus & 0x0f) + 1;
  }
 
  
  bool isChannelMessage(MidiType inType)
  {
     return (inType == NoteOff          ||
            inType == NoteOn            ||
            inType == ControlChange     ||
            inType == AfterTouchPoly    ||
            inType == AfterTouchChannel ||
            inType == PitchBend         ||
            inType == ProgramChange);
   }
 
 
 
bool MIDI_to_BLEMIDI_Parser (void ) { 
 
 
 if (Serial.available() == 0)
        // No data available.
        return false;
 
    // Parsing algorithm:
    // Get a byte from the serial buffer.
    // If there is no pending message to be recomposed, start a new one.
    //  - Find type and channel (if pertinent)
    //  - Look for other bytes in buffer, call parser recursively,
    //    until the message is assembled or the buffer is empty.
    // Else, add the extracted byte to the pending message, and check validity.
    // When the message is done, store it.
 
    const uint8_t extracted = Serial.read();
 
    if (mPendingMessageIndex == 0)
    {
        // Start a new pending message
        mPendingMessage[0] = extracted;
 
        // Check for running status first
        if (isChannelMessage(getTypeFromStatusByte(mRunningStatus_RX)))
        {
            // Only these types allow Running Status
 
            // If the status byte is not received, prepend it
            // to the pending message
            if (extracted < 0x80)
            {
                mPendingMessage[0]   = mRunningStatus_RX;
                mPendingMessage[1]   = extracted;
                mPendingMessageIndex = 1;
            }
            // Else: well, we received another status byte,
            // so the running status does not apply here.
            // It will be updated upon completion of this message.
        }
 
        switch (getTypeFromStatusByte(mPendingMessage[0]))
        {
            // 1 byte messages
            case Start:
            case Continue:
            case Stop:
            case Clock:
            case ActiveSensing:
            case SystemReset:
            case TuneRequest:
            
                // Handle the message type directly here.
                mMessage.type    = getTypeFromStatusByte(mPendingMessage[0]);
                mMessage.channel = 0;
                mMessage.data1   = 0;
                mMessage.data2   = 0;
                mMessage.valid   = true;
 
                // \fix Running Status broken when receiving Clock messages.
                // Do not reset all input attributes, Running Status must remain unchanged.
                //resetInput();
 
                // We still need to reset these
                mPendingMessageIndex = 0;
                mPendingMessageExpectedLenght = 0;
 
                return true;
                break;
 
                // 2 bytes messages
            case ProgramChange:
            case AfterTouchChannel:
            case TimeCodeQuarterFrame:
            case SongSelect:
                mPendingMessageExpectedLenght = 2;
                break;
 
                // 3 bytes messages
            case NoteOn:
            case NoteOff:
            case ControlChange:
            case PitchBend:
            case AfterTouchPoly:
            case SongPosition:
                mPendingMessageExpectedLenght = 3;
                break;
 
            case SystemExclusive:
                // The message can be any lenght
                // between 3 and MidiMessage::sSysExMaxSize bytes
                mPendingMessageExpectedLenght = MidiMessage::sSysExMaxSize;
                mRunningStatus_RX = InvalidType;
                mMessage.sysexArray[0] = SystemExclusive;
                break;
 
            case InvalidType:
            default:
                // This is obviously wrong. Let's get the hell out'a here.
                resetInput();
                return false;
                break;
        }
 
        if (mPendingMessageIndex >= (mPendingMessageExpectedLenght - 1))
        {
            // Reception complete
            mMessage.type    = getTypeFromStatusByte(mPendingMessage[0]);
            mMessage.channel = getChannelFromStatusByte(mPendingMessage[0]);
            mMessage.data1   = mPendingMessage[1];
 
            // Save data2 only if applicable
            if (mPendingMessageExpectedLenght == 3)
                mMessage.data2 = mPendingMessage[2];
            else
                mMessage.data2 = 0;
 
            mPendingMessageIndex = 0;
            mPendingMessageExpectedLenght = 0;
            mMessage.valid = true;
            return true;
        }
        else
        {
            // Waiting for more data
            mPendingMessageIndex++;
        }
 
        if (Use1ByteParsing==1)
        {
            // Message is not complete.
            return false;
        }
        else
        {
            // Call the parser recursively
            // to parse the rest of the message.
            return MIDI_to_BLEMIDI_Parser();
        }
    }
    else
    {
        // First, test if this is a status byte
        if (extracted >= 0x80)
        {
            // Reception of status bytes in the middle of an uncompleted message
            // are allowed only for interleaved Real Time message or EOX
            switch (extracted)
            {
                case Clock:
                case Start:
                case Continue:
                case Stop:
                case ActiveSensing:
                case SystemReset:
 
                    // Here we will have to extract the one-byte message,
                    // pass it to the structure for being read outside
                    // the MIDI class, and recompose the message it was
                    // interleaved into. Oh, and without killing the running status..
                    // This is done by leaving the pending message as is,
                    // it will be completed on next calls.
 
                    mMessage.type    = (MidiType)extracted;
                    mMessage.data1   = 0;
                    mMessage.data2   = 0;
                    mMessage.channel = 0;
                    mMessage.valid   = true;
                    return true;
 
                    break;
 
                    // End of Exclusive
                case 0xf7:
                    if (mMessage.sysexArray[0] == SystemExclusive)
                    {
                        // Store the last byte (EOX)
                        mMessage.sysexArray[mPendingMessageIndex++] = 0xf7;
                        mMessage.type = SystemExclusive;
 
                        // Get length
                        mMessage.data1   = mPendingMessageIndex & 0xff; // LSB
                        mMessage.data2   = mPendingMessageIndex >> 8;   // MSB
                        mMessage.channel = 0;
                        mMessage.valid   = true;
 
                        resetInput();
                        return true;
                    }
                    else
                    {
                        // Well well well.. error.
                        resetInput();
                        return false;
                    }
 
                    break;
                default:
                    break;
            }
        }
 
        // Add extracted data byte to pending message
        if (mPendingMessage[0] == SystemExclusive)
            mMessage.sysexArray[mPendingMessageIndex] = extracted;
        else
            mPendingMessage[mPendingMessageIndex] = extracted;
 
        // Now we are going to check if we have reached the end of the message
        if (mPendingMessageIndex >= (mPendingMessageExpectedLenght - 1))
        {
            // "FML" case: fall down here with an overflown SysEx..
            // This means we received the last possible data byte that can fit
            // the buffer. If this happens, try increasing MidiMessage::sSysExMaxSize.
            if (mPendingMessage[0] == SystemExclusive)
            {
                resetInput();
                return false;
            }
 
            mMessage.type = getTypeFromStatusByte(mPendingMessage[0]);
 
            if (isChannelMessage(mMessage.type))
                mMessage.channel = getChannelFromStatusByte(mPendingMessage[0]);
            else
                mMessage.channel = 0;
 
            mMessage.data1 = mPendingMessage[1];
 
            // Save data2 only if applicable
            if (mPendingMessageExpectedLenght == 3)
                mMessage.data2 = mPendingMessage[2];
            else
                mMessage.data2 = 0;
 
            // Reset local variables
            mPendingMessageIndex = 0;
            mPendingMessageExpectedLenght = 0;
 
            mMessage.valid = true;
 
            // Activate running status (if enabled for the received type)
            switch (mMessage.type)
            {
                case NoteOff:
                case NoteOn:
                case AfterTouchPoly:
                case ControlChange:
                case ProgramChange:
                case AfterTouchChannel:
                case PitchBend:
                    // Running status enabled: store it from received message
                    mRunningStatus_RX = mPendingMessage[0];
                    break;
 
                default:
                    // No running status
                    mRunningStatus_RX = InvalidType;
                    break;
            }
            return true;
        }
        else
        {
            // Then update the index of the pending message.
            mPendingMessageIndex++;
 
            if (Use1ByteParsing==1)
            {
                // Message is not complete.
                return false;
            }
            else
            {
                // Call the parser recursively to parse the rest of the message.
                return MIDI_to_BLEMIDI_Parser();
            }
        }
    }
}
 
 
} // end namespace 
            
