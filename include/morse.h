// Morse codec engine
#ifndef MORSE_H
#define MORSE_H

#include <Arduino.h>
// #include "core.h"
#include "challenger_types.h"

class MorseEngine
{
private:
  word currentCode = 0 ;
  byte currentChar = 0 ;

  byte morseCodeEmitted = 1; // collected morse character for morse decoder
  unsigned long lastElementMs = 0;
  unsigned int asciiToCode(byte ascii);                      // convert printable ASCII char to morse code
  unsigned int utf8ToCode(byte prefix, byte utf8Char);       // convert Ä, Ö, Ü and Cyrillic letters Ч, Ш, Ю and Я

public:
  ElementType getNextElement(); // while playint, provide next element to be played
  void sendAsciiChar(byte ascii);                            // convert ASCII char to morse code and emit
  void cancelSend();
  // void pushElementToDecoder( ElementType element );
  // void sendMorseElement(byte element, bool collect = false); // emit one morse element (dit or dah, followed by pause)
  // void sendString(const char *text); morse engine should send only one character
  // byte getLastCodeFromPaddle(); // collect last morse char played on paddle
  char decodeMorse(word code); // decode morse character played on paddle
};

extern MorseEngine morseEngine;

#endif