#include "keying.h"
#include "paddle.h"

int phase = 0 ;
void setup() {
  keyingInterface.init(); // set up keying interface
  keyingInterface.setTiming(18, 500, 20);
  // keyingInterface.sendElement( DIT );
  paddle.init(); // set up paddle interface
}

void loop() {
  PaddleStatus ps = paddle.check();
  KeyingStatus ks = keyingInterface.service();
  if( ks.busy == IDLE || ks.nextElement == NO_ELEMENT ) {
    if( ps.next != NO_ELEMENT ) {
      ps = paddle.getNext();
      keyingInterface.sendElement( ps.current );
    }
  }
}