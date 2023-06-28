// Morse codec
#ifndef _MORSE_H_
#define _MORSE_H_

#include <Arduino.h>
#include "challenger.h"

class MorseEngine
{
private:
  // word currentCode = 0 ;
  // byte currentChar = 0 ;

  byte morseCodeEmitted = 1; // collected morse character for morse decoder
  // unsigned long lastElementMs = 0;
  byte utf8ToCode(byte prefix, byte utf8Char);       // convert Ä, Ö, Ü and Cyrillic letters Ч, Ш, Ю and Я

public:
  byte asciiToCode(byte ascii); // convert printable ASCII char to morse code
  char decodeMorse(word code);  // decode morse character played on paddle
};

extern MorseEngine morse;

#endif