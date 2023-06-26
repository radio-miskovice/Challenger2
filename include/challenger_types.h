#ifndef _CHALLENGER_TYPES_H_ 
#define _CHALLENGER_TYPES_H_

enum BusyEnum { IDLE = 0, BUSY = 1 };
enum OnOffEnum { OFF = 0, ON = 1};
enum EnableEnum { DISABLED = 0, ENABLED = 1 };
enum ElementType { NO_ELEMENT = 0, DIT = 1, DAH = 2, CHARSPACE = 3, WORDSPACE = 4, HALFSPACE = 5 };
enum PaddleState: byte { PADDLE_FREE = 0, PADDLE_DIT = 1, PADDLE_DAH = 2, PADDLE_SQUEEZE = 3 };

extern unsigned long currentTime ;

#endif