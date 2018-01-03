
////////////////////////////////////////////
// Copyright Sunshine labs
//
// Platform code for the ESP32 board
///////////////////////////////////////////
#ifndef __PLATFORM4_H__
#define __PLATFORM4_H__

// Target platform as C library
typedef unsigned char      bit_t;
typedef unsigned char      u1_t;
typedef   signed char      s1_t;
typedef unsigned short     u2_t;
typedef          short     s2_t;
typedef unsigned int       u4_t;
typedef          int       s4_t;
//typedef unsigned long long u8_t;
//typedef          long long s8_t;
typedef unsigned int       uint;
typedef unsigned short     uint16_t;
typedef unsigned char      uint8_t;
typedef unsigned short     uint16;
typedef          short     int16;
typedef          short     int16_t;
typedef unsigned char      uint8;
typedef signed char        int8;
typedef unsigned int       uint32;
typedef unsigned int       uint32_t;
typedef          int       int32;
typedef          int       int32_t;
typedef const char* str_t;
typedef          uint8     bool;


#include <stdio.h>	
#include "./lio.h"
#include "./cli.h"

#ifndef TICKS_PER_SEC
// #define TICKS_PER_SEC 32768
#define TICKS_PER_SEC 46875
#elif (TICKS_PER_SEC < 10000) || (TICKS_PER_SEC > 64516)
#error Illegal TICKS_PER_SEC - must be in range [10000:64516]. One tick must be 15.5us .. 100us long.
#endif
#define ticks2ms(os)   ((s4_t)(((os)*(s8_t)1000    ) / TICKS_PER_SEC))

#define debug(x) if (gdbf >= x) printf 

char printbuf[128];

//bit routines
/* test if n-th bit in x is set */
#define B_IS_SET(x, n)   (((x) & (1<<(n)))?1:0)

/* set n-th bit in x */
#define B_SET(x, n)      ((x) |= (1<<(n)))

/* unset n-th bit in x */
#define B_CLEAR(x, n)    ((x) &= ~(1<<(n)))

/* toggle n-th bit in x */
#define B_TOGGLE(x, n)   ((x) ^= (1<<(n)))

#define core_1 1
#define core_0 0

typedef void 	(FUNC0)(void);
typedef int		(FUNC1)(char *s);
typedef int 	(*PFUNC0)(void);
typedef void 	(*PFUNC)(void);
typedef int 	(*PFUNC1)(char *s);
typedef int 	(*PFUNC2)(int argc, char *argv[]);
typedef int 	(*FUNC3)(int n);
typedef int 	(*PFUNC3)(int n);
typedef void 	(*FUNC4)(int argc, char *argp);
typedef void 	(*FUNC5)(void);
typedef void 	(FUNC6)(int p);

/////////////////////////////////////////////////////////////////////
// Global console output buffer
/////////////////////////////////////////////////////////////////////
extern char consoleBuff[40];
////////////////////////////////////////////////////////////////////
// External Global data variables
////////////////////////////////////////////////////////////////////
extern uint32 gseconds;   // seconds
extern uint32 lseconds;   //linux time
extern uint32 gmsticks;   // global 1 ms ticks
extern uint32 csticks;     // cycle start ticks for cacluating in millisecs
extern uint32 gwdcount;   // counter for Watch Dog timer
extern uint8  lticks;     // long ticks
extern uint8  gdbf;      // Global debug flag  set in cli.c
extern uint8  gdevid;    //global device number
extern char ghbmess1[]; // heart beat message
extern long glsecs;     // global linux seconds : updated every sec
extern int  gelsecs;    // Global Elapsed seconds since midnight
extern uint8 gperiod;   // period number 1..4
extern uint8 gpcode;    // period code 0..10
//Timer prototypes   
void tg0_timer1_init();

#endif
