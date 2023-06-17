#include "keying.h"

int phase = 0 ;
void setup() {
  keyingInterface.init();
  keyingInterface.setTiming(25, 400, 45);
  keyingInterface.sendElement( DIT );
}

void loop() {
  KeyingStatus s = keyingInterface.service() ;
  ElementType e ;

  if( phase > 48 ) return ;
  if( s.busy == IDLE || s.nextElement == NO_ELEMENT ) {
    switch (phase++ % 4)
    {
    case 0:
      e = DIT;
      break;
    case 1:
      e = DAH;
      break;
    case 2:
      e = CHARSPACE;
      break;
    case 3:
      e = NO_ELEMENT;
    }
    keyingInterface.sendElement(e);
  }
}