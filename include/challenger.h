#ifndef _CHALLENGER_H_ 
#define _CHALLENGER_H_

/**
 * Global types and variables for Challenger 
 * and all functional components.
 */

enum BusyEnum    { READY = 0, BUSY = 1 };
enum OnOffEnum   { OFF = 0, ON = 1};
enum EnableEnum  { DISABLED = 0, ENABLED = 1 };
enum ElementType { NO_ELEMENT = 0, DIT = 1, DAH = 2, CHARSPACE = 3, WORDSPACE = 4, HALFSPACE = 5 };
enum PaddleState { PADDLE_FREE = 0, PADDLE_DIT = 1, PADDLE_DAH = 2, PADDLE_SQUEEZE = 3 };
enum YesNoEnum   { NO = 0, YES = 1 };

const byte MORSE_SPACE = 0xFF;
const byte MORSE_CHARSPACE = 0x80;

extern unsigned long currentTime ;

#if !defined( LED_BUILTIN )
#define LED_BUILTIN 13
#endif

#endif