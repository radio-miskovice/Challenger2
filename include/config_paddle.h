#ifndef _CONFIG_PADDLE_H_
#define _CONFIG_PADDLE_H_

#if defined(HW_CHALLENGER_PLAST)
// Left paddle pin: by standard it is tip contact on TRS connector
#define CONFIG_PADDLE_LEFT  2
// Right paddle pin: by standard it is ring contact on TRS connector
#define CONFIG_PADDLE_RIGHT 3
#elif defined(HW_CHALLENGER2)
#define CONFIG_PADDLE_LEFT D3
// Right paddle pin: by standard it is ring contact on TRS connector
#define CONFIG_PADDLE_RIGHT D2

#else 

#error You have no HW configuration for PADDLE

#endif 

#endif