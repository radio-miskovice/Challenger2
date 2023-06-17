#include <Arduino.h>
#include "config_keying.h"
#include "challenger_types.h"

//typedef
enum ElementType { NO_ELEMENT = 0, DIT = 1, DAH = 2, CHARSPACE = 3, WORDSPACE = 4 } ;


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
  ElementType currentElement: 3; 
  ElementType nextElement: 3;    
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
  word unit = 50;          // default timing unit is 50 msec = 24 WPM
  word weighting = 50 ;    // DIT duration in percent, element space is then 100 - weighting
  word ditDahFactor = 300 ;   // DAH duration in percent of DIT element time including weighting
  word toneFreq = 600 ;    // default sidetone frequency
 
  // timing variables
  unsigned long onTimer; // countdown timer for mark time in high-level sending
  unsigned long offTimer;    // countdown timer for space time in high-level sending
  unsigned long lastMillis ; // millisecond CPU time during the last service tick

  // private methods
  word trimToneFreq( word hz ); // trim tone frequency to stay between limits or keep zero
  void setKey(OnOffEnum onOff); // low-level key control
  void setPtt(OnOffEnum onOff); // low-level PTT control
  void newCurrentElement(ElementType element); // set status, onTimer and offTimer accordingly

public:
  void init();  // port setup
  void enableKey(EnableEnum enable); // enable or disable KEY output
  void enablePtt(EnableEnum enable); // enable or disable PTT output
  void enableTone(EnableEnum enable); // enable or disable tone
  void setTone(word hz);   // low-level sidetone control
  void setTiming(byte wpm, word aDahRatio = 0, word aWeighting = 0); // set time constants for given WPM speed, DAH:DIT ratio and weighting
  void setToneFreq(word hz);                                 // set tone frequency for high-level sending
  KeyingStatus sendElement( ElementType element ); // start element immediately or put it in queue
  KeyingStatus service();  // read current millis, update timers, ports and status accordingly and return new service status
};

extern KeyingInterface keyingInterface;
