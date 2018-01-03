
#ifndef LEDFLASHREQUIRED_H_
#define LEDFLASHREQUIRED_H_

#include "sc_types.h"
#include "Ledflash.h"

#ifdef __cplusplus
extern "C"
{
#endif 

/*! \file This header defines prototypes for all functions that are required by the state machine implementation.

This is a state machine uses time events which require access to a timing service. Thus the function prototypes:
	- ledflash_setTimer and
	- ledflash_unsetTimer
are defined.

This state machine makes use of operations declared in the state machines interface or internal scopes. Thus the function prototypes:
	- ledflashIface_blink
	- ledflashIface_ledon
	- ledflashIface_ledoff
	- ledflashIface_notifyOn
	- ledflashIface_notifyOff
	- ledflashIface_notifyIdle
are defined.

These functions will be called during a 'run to completion step' (runCycle) of the statechart. 
There are some constraints that have to be considered for the implementation of these functions:
	- never call the statechart API functions from within these functions.
	- make sure that the execution time is as short as possible.
 
*/
extern void ledflashIface_blink(const Ledflash* handle);
extern void ledflashIface_ledon(const Ledflash* handle);
extern void ledflashIface_ledoff(const Ledflash* handle);
extern void ledflashIface_notifyOn(const Ledflash* handle);
extern void ledflashIface_notifyOff(const Ledflash* handle);
extern void ledflashIface_notifyIdle(const Ledflash* handle);


/*!
 * This is a timed state machine that requires timer services
 */ 

/*! This function has to set up timers for the time events that are required by the state machine. */
/*! 
	This function will be called for each time event that is relevant for a state when a state will be entered.
	\param evid An unique identifier of the event.
	\time_ms The time in milli seconds
	\periodic Indicates the the time event must be raised periodically until the timer is unset 
*/
extern void ledflash_setTimer(Ledflash* handle, const sc_eventid evid, const sc_integer time_ms, const sc_boolean periodic);

/*! This function has to unset timers for the time events that are required by the state machine. */
/*! 
	This function will be called for each time event taht is relevant for a state when a state will be left.
	\param evid An unique identifier of the event.
*/
extern void ledflash_unsetTimer(Ledflash* handle, const sc_eventid evid);



#ifdef __cplusplus
}
#endif 

#endif /* LEDFLASHREQUIRED_H_ */
