#include <Arduino.h>
// #include "types.h"
#include "keying.h"

// Keying interface singleton
KeyingInterface keyingInterface = KeyingInterface() ;

/**
 * initializes Arduino ports
 */
void KeyingInterface::init()
{
  if (_keyline1 >0)  pinMode(_keyline1, OUTPUT);
  if (_pttline1 > 0) pinMode(_pttline1, OUTPUT);
  if (_sidetone > 0) pinMode(_sidetone, OUTPUT);
  if (_cpo_key > 0)  pinMode(_cpo_key, OUTPUT);
}

/**
 * set Key line.
 * @param level - high or low
 */
void KeyingInterface::setKey(byte level)
{
  digitalWrite(_keyline1, level);
}

/**
 * @param level - high or low
*/
void KeyingInterface::setPtt(byte level)
{
  digitalWrite(_pttline1, level);
}

/**
 * @param hz when hz = 0 stops sidetone, otherwise starts sidetone with frequency hz Hz
*/
void KeyingInterface::setTone(word hz)
{
  if(hz > 0) {
    if( hz < minToneFreq ) hz = minToneFreq ;
    else if (hz > maxToneFreq ) hz = maxToneFreq ;
    tone(_sidetone, hz); 
  }
  else noTone(_sidetone);
}
