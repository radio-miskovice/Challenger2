#include <Arduino.h>
#include "config_keying.h"


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

  word dashDotRatio = 300; // default dash:dot timing is 3:1
  word unit = 50;          // default timing unit is 50 msec

public:
  void init();  // port setup
  void setKey(byte level); // low-level key control
  void setPtt(byte level); // low-level PTT control
  void setTone(word hz);   // low-level sidetone control
};

extern KeyingInterface keyingInterface;
