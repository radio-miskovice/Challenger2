#include <Arduino.h>
#include "config_keying.h"

//typedef
enum ElementType : byte { NO_ELEMENT = 0, DIT = 1, DAH = 2, CHARSPACE = 3, WORDSPACE = 4 } ;
enum KeyerBusyEnum: byte { IDLE = 0, BUSY = 1 };
enum KeyerLineStatusEnum : byte {   OFF = 0,  ON = 1 };
enum KeyerEnableEnum: byte { DISABLED = 0, ENABLED = 1 };
struct KeyingFlags
{
  KeyerEnableEnum tone : 1;
  KeyerEnableEnum ptt : 1;
  KeyerEnableEnum key : 1;
} ;

struct KeyingStatus {
  KeyerBusyEnum busy : 1;  // 1 if busy in timing process, 0 when idle
  KeyerLineStatusEnum force : 1; // 1 if PTT or KEY is forced (ie. no element timing)
  KeyerLineStatusEnum ptt : 1;   // 1 if PTT active
  KeyerLineStatusEnum key : 1;   // 1 if KEY active
  ElementType currentElement: 3; // current element in progress, see enum definition
  ElementType nextElement: 3;    // next element to follow after current one 
};

class KeyingInterface
{

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
  KeyingStatus status = { busy : IDLE, force : OFF, ptt : OFF, key : OFF, currentElement : NO_ELEMENT, nextElement: NO_ELEMENT };

  // keying parameter settings 
  word dashDotRatio = 300; // default dash:dot timing is 3:1
  word unit = 50;          // default timing unit is 50 msec = 24 WPM
  word weighting = 50 ;    // DIT duration in percent, element space is then 100 - weighting
  word dahFactor = 300 ;    // DAH duration in percent of DIT element time including weighting
  word toneFreq = 600 ;    // default sidetone frequency
 
  // timing variables
  unsigned long onTimer; // countdown timer for mark time in high-level sending
  unsigned long offTimer;    // countdown timer for space time in high-level sending
  unsigned long lastMillis ; // millisecond CPU time during the last service tick

  // private methods
  void newCurrentElement(ElementType element); // set status, onTimer and offTimer accordingly

public:
  void init();  // port setup
  void enableKey(KeyerEnableEnum enable); // enable or disable KEY output
  void enablePtt(KeyerEnableEnum enable); // enable or disable PTT output
  void enableTone(KeyerEnableEnum enable); // enable or disable tone
  void setKey(KeyerLineStatusEnum onOff); // low-level key control
  void setPtt(KeyerLineStatusEnum onOff); // low-level PTT control
  void setTone(word hz);   // low-level sidetone control
  void setTiming(byte wpm, word aDahRatio = 0, word aWeighting = 0); // set time constants for given WPM speed, DAH:DIT ratio and weighting
  void setToneFreq(word hz);                                 // set tone frequency for high-level sending
  word trimToneFreq( word hz ); // trim tone frequency to stay between limits or keep zero
  KeyingStatus sendElement( ElementType element ); // start element immediately or put it in queue
  KeyingStatus service();  // read current millis, update timers, ports and status accordingly and return new service status
};

extern KeyingInterface keyingInterface;
