#include "keying.h"
#include "paddle.h"
#include "speed_control.h"
#include "protocol.h"

// debugging
unsigned long blikTime = 0 ;
const byte LED = A0 ;
void blik(bool);

// data type

/* GLOBAL VARIABLES */
KeyingSource keySource = SRC_PADDLE ;
int speedPaddles = 25 ;
int speedBuffer = 20 ;
int speedCommand = 20 ;
unsigned long currentTime ;

void setup() {
  protocol.init();
  keyer.init();
  keyer.setMode(IAMBIC_B);
  keyer.setTimingParameters(speedPaddles, 300, 50);
  keyer.enableTone(ENABLED);
  paddle.init();
  speedControl->init();
  // command mode LED
  pinMode( LED, OUTPUT );
  // debugging
  keyer.setTone(300);
  digitalWrite( LED, HIGH );
  delay(133);
  keyer.setTone(1300);
  delay(133);
  digitalWrite( LED, LOW );
  keyer.setTone(0);
  delay(100);
  currentTime = millis();
  keyer.service(0);
  keyer.sendCode(0b10101000);
  keyer.sendCode(0b00111100);
  Serial.begin(1200);

}

char message[100];
// char qbf[] = "= QUICK BROWN FOX JUMPS OVER THE LAZY DOG.  / 1234567890 + ";
int index = 0 ;
KeyingStatus kkk ;

union B {
  KeyingStatus s;
  word w ;
} current, previous ;

void loop() {
  currentTime = millis();
  speedControl->update();
  int speed = speedControl->getValue();
  if( speed != speedPaddles ) {
    speedPaddles = speed ;
    keyer.setTimingParameters( speedPaddles );
    blik(true);
  }
  else blik(false);
  protocol.service(); // check incoming data and execute command if necessary
  byte paddleState = paddle.check();
  KeyingStatus keyerState = keyer.service( paddleState ) ; // check and update timing and status
  current.s = keyerState ;
  if( keyerState.breakIn == ON ) {
    protocol.stopBuffer() ;
  }
  else if( keyerState.buffer == ENABLED ) { // can send from buffer?
    byte x ;
    // x = protocol.getNextMorseCode();
    // Serial.println( x, 2 ); 
    // keyerState = keyer.sendCode( x ); // send code or do nothing if got zero
  }
  // protocol.setStatus( keyerState, paddleState );
  // protocol.sendStatus();
}

// for debugging; remove in production code
void blik(bool start) {
  if( start ) {
    blikTime = currentTime + 25;
    digitalWrite(LED, HIGH);
  }
  else if( blikTime < currentTime ) {
    digitalWrite( LED, LOW );
  }
}
