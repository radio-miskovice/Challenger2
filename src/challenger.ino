#include "keying.h"

int phase = 0 ;
void setup() {
  keyingInterface.init();
}

void loop() {
  if( phase > 100 ) return; 
  int bip = 1 & phase++ ;
  keyingInterface.setTone( bip * 1000 );
  delay(50);
}