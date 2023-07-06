#ifndef _KEYING_H_
#define _KEYING_H_

#include <Arduino.h>
#include "config_keying.h"
#include "challenger.h"

//typedef
struct KeyingFlags
{
  EnableEnum tone : 1;
  EnableEnum ptt : 1;
  EnableEnum key : 1;
  EnableEnum autospace: 1 ;
  EnableEnum contestSpacing: 1 ;
} ;

enum KeyerMode
{
  IAMBIC_A = 1,
  IAMBIC_B = 2,
  ULTIMATIC = 3
};

enum KeyingSource
{
  SRC_PADDLE,
  SRC_BUFFER,
  SRC_COMMAND
};

// the following structure is designed to fit bitwise into WK2 status
struct KeyerState {
  OnOffEnum  key : 1;       // will be replaced by XOFF bit in WK2 status byte
  OnOffEnum  breakIn : 1;   // WK1, WK2 BREAKIN bit
  OnOffEnum  bufferSend: 1; // WK1, WK2 BUSY bit
  EnableEnum accept : 1;    // will be masked off in WK2 status byte
  BusyEnum   busy : 1;      // not sure if correct; WK1, WK2 WAIT bit
  KeyingSource source : 2;  // will be masked off in WK2 status byte
  OnOffEnum force : 1;      // will be masked off in WK2 status byte
  OnOffEnum ptt : 1;        // will be masked off in WK2 status byte
  KeyerMode mode : 2;       // will be masked off in WK2 status byte
  YesNoEnum hasPaddleCode: 1; // YES when a code is ready 
};

struct InternalStatus {
  ElementType current: 3; 
  ElementType last: 3;
};

// union KeyerStateWord {
//   KeyerState state ;
//   word w ;
// };

class KeyingInterface {
private:
  // keying interface object is supposed to be used as singleton, hence we use static constants
  static const byte pin_keyline1 = CONFIG_KEYING_KEYLINE1; // key line, active HIGH
  static const byte pin_pttline1 = CONFIG_KEYING_PTTLINE1; // PTT line, active HIGH
  static const byte pin_sidetone = CONFIG_KEYING_SIDETONE; // sidetone out AC
  static const byte pin_cpo_key  = CONFIG_KEYING_CPO;      // sidetone keying, active high

  static const word minToneFreq = CONFIG_SIDETONE_MIN_FREQ; // minimum sidetone frequency
  static const word maxToneFreq = CONFIG_SIDETONE_MAX_FREQ; // maximum sidetone frequency

  // operational parameters, internal parameters and status
  KeyingFlags flags = { tone: ENABLED, ptt: ENABLED, key: ENABLED, autospace: DISABLED, contestSpacing: DISABLED };
  KeyerState status = { 
    key : OFF, 
    breakIn : OFF ,
    bufferSend: OFF,
    accept : ENABLED, 
    busy : READY, 
    source : SRC_PADDLE, 
    force : OFF, 
    ptt : OFF,
    mode : IAMBIC_A,
    hasPaddleCode: NO
  };

  InternalStatus internal = { current : NO_ELEMENT, last: NO_ELEMENT };

  // binary morse code buffer memory
  byte currentMorse = 0 ;
  byte nextMorse = 0 ;

  // keying parameter settings 
  word unit = 50;           // default timing unit is 50 msec = 24 WPM
  word weighting = 50 ;     // DIT duration in percent, element space is then 100 - weighting
  word ditDahFactor = 300 ; // DAH duration in percent of DIT element time including weighting
  word toneFreq = 600 ;     // default sidetone frequency
  byte farnsWorthWpm = 10 ; // Farnsworth speed 
  byte pttLead = 0, pttTail = 0; 
  byte firstExtension = 0 ;
  byte qskCompensation = 0 ;

  // internal memory to handle paddle input
  byte paddleMemUltimatic = PADDLE_FREE ;  // remember last ultimatic decision 
  byte paddleMemory = PADDLE_FREE ; // paddle memory for Iambic B

  // internal memory to collect paddle keying for decode
  word morseCollector = 0 ; // buffer memory to hold current morse elements
  word morseCollected = 0 ; // final morse code after it has been finished
  //byte asciiCollected = 0 ;
 
  // timing variables
  unsigned long onTimer; // countdown timer for mark time in high-level sending
  unsigned long offTimer;    // countdown timer for space time in high-level sending
  unsigned long lastMillis ; // millisecond CPU time during the last service tick
  unsigned long hardKeyTimeout = 0 ;
  unsigned long collectionTimeout = 0 ;

  // private methods
  word trimToneFreq(word hz);   // trim tone frequency to stay between limits or keep zero
  void setKey(OnOffEnum onOff); // low-level key control
  void setPtt(OnOffEnum onOff); // low-level PTT control
  KeyerState handleBreakIn() ;  // all necessary actions to set break-in condition
  void collectPaddleElement( ElementType element );

public:
  void init();  // port setup
  bool canAccept(); // true if a binary morse code can be received by internal keying buffer
  void enableKey(EnableEnum enable);  // enable or disable KEY output
  void enablePtt(EnableEnum enable);  // enable or disable PTT output
  void enableTone(EnableEnum enable); // enable or disable tone
  word getCollectedCode(); // return collected morse code if available
  KeyerState getState() ; 
  void setAutospace(EnableEnum enable ); // action to respond to protocol command
  void setDefaults();                    // set default parameters
  void setFarnsworthWpm(byte wpm);       // action to respond to protocol command
  void setFirstExtension(byte ms);       // action to respond to protocol command
  void setKey(OnOffEnum onOff, word timeout); // low-level key control
  void setMode(KeyerMode newMode);            // action to respond to protocol command
  void setPttTiming(byte lead, byte tail);    // action to respond to protocol command
  void setQskCompensation(byte ms);           // action to respond to protocol command
  void setSource( KeyingSource );  // set source accordingly
  void setTimingParameters(byte wpm, word aDahRatio = 0, word aWeighting = 0); // set time constants for given WPM speed, DAH:DIT ratio and weighting
  void setTone(word hz);      // low-level sidetone control
  void setToneFreq(word hz);  // set tone frequency for high-level sending
  void sendElement(ElementType element); // set status, onTimer and offTimer accordingly
  void sendPaddleElement( byte ); // determine element from paddle input and mode, and start sending
  KeyerState sendCode( byte );  // send binary morse code; return true if accepted, false otherwise
  KeyerState service( byte );   // read current millis, update timers, ports and status accordingly and return new service status
};

extern KeyingInterface keyer;
#endif