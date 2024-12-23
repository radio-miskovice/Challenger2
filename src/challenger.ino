#include "keying.h"
#include "paddle.h"
#include "speed_control.h"
#include "protocol.h"
#include "morse.h"

// debugging
unsigned long blikTime = 0 ;
const byte LED = CONFIG_CMD_MODE_LED ;
void blik(bool);

/* GLOBAL VARIABLES */
KeyingSource keySource = SRC_PADDLE ;
int speedPaddles = 25 ;
int speedBuffer = 20 ;
int speedCommand = 20 ;
unsigned long currentTime ;

void setup() {
  protocol.init();
  // BUFFER indicator setup
  pinMode( LED_BUILTIN, OUTPUT );
  digitalWrite( LED_BUILTIN, HIGH );
  // basic component setup
  keyer.init();
  keyer.setDefaults();
  paddle.init();
  speedControl->init(); // includes also command mode LED
  speedControl->setMinMax(15,46);
  speedControl->setValue(speedPaddles);
   // initial beep and flash
  keyer.setTone(300);
  digitalWrite( LED, HIGH );
  delay(133);
  keyer.setTone(1300);
  delay(133);
  digitalWrite( LED, LOW );
  keyer.setTone(0);
  delay(100);
  // initial
  currentTime = millis();
  keyer.service(0);
  digitalWrite( LED_BUILTIN, LOW );
  // testing parameters
  protocol.enablePaddleEcho( ON );
}

void loop() {
  // fix current time at the beginning of the loop
  currentTime = millis();
  // let speed control update current value if anything changed by ISR
  speedControl->update();
  int speed = speedControl->getValue();
  // update keyer timing according to new value from speed control
  if( speed != speedPaddles ) {
    speedPaddles = speed ;
    keyer.setTimingParameters( speedPaddles );
    blik(true);
    protocol.sendResponse( speedControl->getSpeedWk2() ); // send WK status speed info if speed changed
  }
  else blik(false); // this ensures LED flash when speed is changed
  // check current paddle state (just read ports, nothing else)
  byte paddleState = paddle.check();
  // Service one tick in timing (key down, sidetone, pause between elements). 
  // Variable paddleState is used to determine the next element if necessary. 
  KeyerState keyerState = keyer.service( paddleState ) ; // for details see keying.cpp
  protocol.service(keyerState); // Check incoming serial data and execute command if necessary
  // The following block will fetch next morse code into keyer if keyer ready and morse code available from buffer
  if( keyer.canAccept() ) 
  { 
     byte x = protocol.getNextMorseCode(); // also send new status re XON, XOFF; returns 0 if nothing available in the buffer
     keyerState = keyer.sendCode( x );     // send obtained morse code; does nothing if code is zero
  }
  // The following block retrieves morse code just played on paddles and converts to ASCII char
  if( keyerState.source == SRC_PADDLE && keyerState.busy == READY ) {
    word code = keyer.getCollectedCode(); // keyer timing also detects word space and returns special code if detected
    byte ascii = morse.decodeMorse(code);
    if( ascii >= ' ' ) protocol.sendPaddleEcho(ascii); // this actually sends echo only if enabled and character makes sense
  }
  protocol.sendStatus(keyerState); // after all functions have been serviced, send new Winkeyer status if Winkeyer status changed
}

// speed change indicator
void blik(bool start) {
  if( start ) {
    blikTime = currentTime + 5;
    digitalWrite(LED, HIGH);
  }
  else if( blikTime < currentTime ) {
    digitalWrite( LED, LOW );
  }
}