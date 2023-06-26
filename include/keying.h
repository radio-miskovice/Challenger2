#include <Arduino.h>
#include "config_keying.h"
#include "challenger_types.h"

//typedef
struct KeyingFlags
{
  EnableEnum tone : 1;
  EnableEnum ptt : 1;
  EnableEnum key : 1;
} ;

struct KeyingStatus {
  BusyEnum busy : 1;        
  OnOffEnum force : 1; 
  OnOffEnum ptt : 1;   
  OnOffEnum key : 1;   
  ElementType current: 3; 
  ElementType last: 3;
};

enum KeyerModeEnum
{
  IAMBIC_A = 0,
  IAMBIC_B = 1,
  ULTIMATIC = 2
};

enum KeyingSource
{
  SRC_PADDLE,
  SRC_BUFFER,
  SRC_COMMAND
};

class KeyingInterface {

private:
  // keying interface object is supposed to be used as singleton, hence we use static constants
  static const byte _keyline1 = CONFIG_KEYING_KEYLINE1; // key line, active HIGH
  static const byte _pttline1 = CONFIG_KEYING_PTTLINE1; // PTT line, active HIGH
  static const byte _sidetone = CONFIG_KEYING_SIDETONE; // sidetone out AC
  static const byte _cpo_key = CONFIG_KEYING_CPO;       // sidetone keying, active high

  static const word minToneFreq = CONFIG_SIDETONE_MIN_FREQ; // minimum sidetone frequency
  static const word maxToneFreq = CONFIG_SIDETONE_MAX_FREQ; // maximum sidetone frequency

  // operational parameters and status
  KeyingFlags flags = { tone: ENABLED, ptt: ENABLED, key: ENABLED };
  KeyingStatus status = { busy : IDLE, force : OFF, ptt : OFF, key : OFF, current : NO_ELEMENT, last: NO_ELEMENT };
  KeyerModeEnum mode = IAMBIC_A ;
  KeyingSource currentSource = SRC_PADDLE ;

  // keying parameter settings 
  word unit = 50;          // default timing unit is 50 msec = 24 WPM
  word weighting = 50 ;    // DIT duration in percent, element space is then 100 - weighting
  word ditDahFactor = 300 ;     // DAH duration in percent of DIT element time including weighting
  word toneFreq = 600 ;         // default sidetone frequency
  byte lastUltimaticPaddle = PADDLE_FREE ;  // remember last ultimatic decision 
  byte lastBPaddle = PADDLE_FREE ; // paddle memory for Iambic B
 
  // timing variables
  unsigned long onTimer; // countdown timer for mark time in high-level sending
  unsigned long offTimer;    // countdown timer for space time in high-level sending
  unsigned long lastMillis ; // millisecond CPU time during the last service tick

  // private methods
  word trimToneFreq( word hz ); // trim tone frequency to stay between limits or keep zero
  void setKey(OnOffEnum onOff); // low-level key control
  void setPtt(OnOffEnum onOff); // low-level PTT control

public:
  void init();  // port setup
  void enableKey(EnableEnum enable); // enable or disable KEY output
  void enablePtt(EnableEnum enable); // enable or disable PTT output
  void enableTone(EnableEnum enable); // enable or disable tone
  void setMode(KeyerModeEnum newMode);
  void setTone(word hz);   // low-level sidetone control
  void setTimingParameters(byte wpm, word aDahRatio = 0, word aWeighting = 0); // set time constants for given WPM speed, DAH:DIT ratio and weighting
  void setToneFreq(word hz);                       // set tone frequency for high-level sending
  KeyingStatus sendElement(ElementType element, bool addCharSpace = false); // set status, onTimer and offTimer accordingly
  // KeyingStatus sendElement( ElementType element, bool isEndOfChar = false ); // explicit element, play immediately 
  KeyingStatus sendPaddleElement( byte ); // determine element from paddle input and mode, and play it
  KeyingStatus service( byte ); // read current millis, update timers, ports and status accordingly and return new service status
  // void checkPaddle( byte ); // check paddle status before end of element
  // DEBUGGING 
  unsigned long getOnTime();
  unsigned long getOffTime();
};

extern KeyingInterface keyer;
