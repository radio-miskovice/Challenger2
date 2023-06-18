#include "keying.h"
#include "paddle.h"

int phase = 0 ;
void setup() {
  keyingInterface.init();
  keyingInterface.setTimingParameters(18, 300, 50);
  keyingInterface.sendElement(DIT);
  keyingInterface.sendElement(DAH);
  delay(500);

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