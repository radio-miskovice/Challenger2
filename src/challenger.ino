#include "keying.h"
#include "paddle.h"
#include "speed_control.h"
#include "morse.h"

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
unsigned long nextBufferTime;

void setup() {
  currentTime = millis();
  pinMode( LED, OUTPUT );
  keyer.init();
  keyer.setMode(IAMBIC_B);
  keyer.setTimingParameters(speedPaddles, 300, 50);
  paddle.init();
  speedControl->init();
  keyer.enableTone(ENABLED);
  // debugging
  Serial.begin(57600);
  keyer.setTone(300);
  digitalWrite( LED, HIGH );
  delay(133);
  keyer.setTone(1300);
  delay(133);
  digitalWrite( LED, LOW );
  keyer.setTone(0);
  nextBufferTime = millis();
}

char message[100];
char qbf[] = "= QUICK BROWN FOX JUMPS OVER THE LAZY DOG.  / 1234567890 + ";
int index = 0 ;

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
  // paddle
  byte paddleState = paddle.check();
  KeyingStatus keyerState = keyer.service( paddleState ) ; // check and update timing and status
  // morse engine - start if not sending from paddles for 3 seconds
  if( nextBufferTime < currentTime ) {
    if( paddleState == 0 && keyerState.buffer == ENABLED ) {
      char c ;
      byte code ;
      do {
        c = qbf[index++];
        if( c == 0 ) index = 0 ;
      } while ( c == 0 );
      code = morse.asciiToCode( c );
      if( code ) keyer.sendCode( code );
    }
  }
  if( paddleState > 0 ) {nextBufferTime = currentTime + 3000UL ; index = 0 ; } // if paddle touched, start buffer text after 3 secs
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
