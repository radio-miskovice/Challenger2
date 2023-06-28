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

struct KeyingStatus {
  KeyingSource source : 2 ; 
  KeyerMode  mode : 2 ;
  EnableEnum buffer : 1 ;
  BusyEnum   busy : 1;        
  OnOffEnum  force : 1; 
  OnOffEnum  ptt : 1;   
  OnOffEnum  key : 1;  
  OnOffEnum  breakIn : 1 ;
};

struct InternalStatus {
  ElementType current: 3; 
  ElementType last: 3;
};

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
  KeyingFlags flags = { tone: ENABLED, ptt: ENABLED, key: ENABLED };
  KeyingStatus status = { source : SRC_PADDLE, mode : IAMBIC_A, buffer : ENABLED, busy : READY, force : OFF, ptt : OFF, key : OFF, breakIn : OFF };
  InternalStatus internal = { current : NO_ELEMENT, last: NO_ELEMENT };
  byte currentMorse = 0 ;
  byte nextMorse = 0 ;

  // binary morse code buffer memory

  // keying parameter settings 
  word unit = 50;           // default timing unit is 50 msec = 24 WPM
  word weighting = 50 ;     // DIT duration in percent, element space is then 100 - weighting
  word ditDahFactor = 300 ; // DAH duration in percent of DIT element time including weighting
  word toneFreq = 600 ;     // default sidetone frequency
  byte farnsWorthWpm = 10 ; // Farnsworth speed 
  byte pttLead = 0, pttTail = 0; 
  byte firstExtension = 0 ;
  byte qskCompensation = 0 ;
  byte paddleMemUltimatic = PADDLE_FREE ;  // remember last ultimatic decision 
  byte paddleMemory = PADDLE_FREE ; // paddle memory for Iambic B
 
  // timing variables
  unsigned long onTimer; // countdown timer for mark time in high-level sending
  unsigned long offTimer;    // countdown timer for space time in high-level sending
  unsigned long lastMillis ; // millisecond CPU time during the last service tick
  unsigned long hardKeyTimeout = 0 ;

  // private methods
  word trimToneFreq(word hz);   // trim tone frequency to stay between limits or keep zero
  void setKey(OnOffEnum onOff); // low-level key control
  void setPtt(OnOffEnum onOff); // low-level PTT control

public:
  void init();  // port setup
  void enableKey(EnableEnum enable);  // enable or disable KEY output
  void enablePtt(EnableEnum enable);  // enable or disable PTT output
  void enableTone(EnableEnum enable); // enable or disable tone
  void setBreakIn() ;
  void setFarnsworthWpm( byte wpm );
  void setFirstExtension( byte ms );
  void setKey(OnOffEnum onOff, word timeout); // low-level key control
  void setMode(KeyerMode newMode);
  void setPttTiming( byte lead, byte tail );
  void setQskCompensation( byte ms );
  void setSource( KeyingSource );
  void setTimingParameters(byte wpm, word aDahRatio = 0, word aWeighting = 0); // set time constants for given WPM speed, DAH:DIT ratio and weighting
  void setTone(word hz);      // low-level sidetone control
  void setToneFreq(word hz);  // set tone frequency for high-level sending
  void sendElement(ElementType element); // set status, onTimer and offTimer accordingly
  void sendPaddleElement( byte ); // determine element from paddle input and mode, and start sending
  KeyingStatus sendCode( byte );  // send binary morse code; return true if accepted, false otherwise
  KeyingStatus service( byte );   // read current millis, update timers, ports and status accordingly and return new service status
};

extern KeyingInterface keyer;
#endif