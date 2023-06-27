#ifndef BUFFER_H
#define BUFFER_H

#include <Arduino.h>

/** FIFO class to implement circular buffer **/
class CharacterFIFO
{
private:
  char buffer[256];
  byte head = 0;
  byte tail = 0;

public:
  void reset();      // reset buffer content - empty buffer
  void push(char x); // push new character at the end of buffer
  char shift();      // read character from
  void unshift();
  byte getLength();
  byte getFree();
  bool hasMore();
  bool canTake();
};

#endif