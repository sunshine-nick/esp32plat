
#ifndef LEDFLASH_H_
#define LEDFLASH_H_

#include "sc_types.h"
		
#ifdef __cplusplus
extern "C" { 
#endif 

/*! \file Header of the state machine 'ledflash'.
*/


#ifndef LEDFLASH_EVENTQUEUE_BUFFERSIZE
#define LEDFLASH_EVENTQUEUE_BUFFERSIZE 20
#endif

/*
 * Enum of event names in the statechart.
 */
typedef enum  {
	invalid_event,
	ledflashIface_turnon,
	ledflashIface_turnoff
} ledflash_event_name;

/*
 * Union of all possible event value types.
 */
typedef union {
} ledflash_event_value;

/*
 * Struct that represents a single event.
 */
typedef struct {
	ledflash_event_name name;
	sc_boolean has_value;
	ledflash_event_value value;
} ledflash_event;

/*
 * Queue that holds the raised events.
 */
typedef struct ledflash_eventqueue_s {
	ledflash_event events[LEDFLASH_EVENTQUEUE_BUFFERSIZE];
	sc_integer pop_index;
	sc_integer push_index;
	sc_integer size;
} ledflash_eventqueue;

void ledflash_event_init(ledflash_event * ev, ledflash_event_name name);

void ledflash_event_value_init(ledflash_event * ev, ledflash_event_name name, void * value);

void ledflash_eventqueue_init(ledflash_eventqueue * eq);

sc_integer ledflash_eventqueue_size(ledflash_eventqueue * eq);

ledflash_event ledflash_eventqueue_pop(ledflash_eventqueue * eq);

sc_boolean ledflash_eventqueue_push(ledflash_eventqueue * eq, ledflash_event ev);

/*! Enumeration of all states */ 
typedef enum
{
	Ledflash_last_state,
	Ledflash_main_region_main,
	Ledflash_main_region_main_r1_idle,
	Ledflash_main_region_main_r1_flash,
	Ledflash_main_region_pause
} LedflashStates;

/*! Type definition of the data structure for the LedflashIface interface scope. */
typedef struct
{
	sc_boolean turnon_raised;
	sc_boolean turnoff_raised;
} LedflashIface;

/*! Type definition of the data structure for the LedflashTimeEvents interface scope. */
typedef struct
{
	sc_boolean ledflash_main_region_main_r1_idle_tev0_raised;
	sc_boolean ledflash_main_region_main_r1_flash_tev0_raised;
} LedflashTimeEvents;


/*! Define dimension of the state configuration vector for orthogonal states. */
#define LEDFLASH_MAX_ORTHOGONAL_STATES 1

/*! Define indices of states in the StateConfVector */
#define SCVI_LEDFLASH_MAIN_REGION_MAIN 0
#define SCVI_LEDFLASH_MAIN_REGION_MAIN_R1_IDLE 0
#define SCVI_LEDFLASH_MAIN_REGION_MAIN_R1_FLASH 0
#define SCVI_LEDFLASH_MAIN_REGION_PAUSE 0

/*! 
 * Type definition of the data structure for the Ledflash state machine.
 * This data structure has to be allocated by the client code. 
 */
typedef struct
{
	LedflashStates stateConfVector[LEDFLASH_MAX_ORTHOGONAL_STATES];
	sc_ushort stateConfVectorPosition; 
	
	LedflashIface iface;
	LedflashTimeEvents timeEvents;
	ledflash_eventqueue internal_event_queue;
} Ledflash;


/*! Initializes the Ledflash state machine data structures. Must be called before first usage.*/
extern void ledflash_init(Ledflash* handle);

/*! Activates the state machine */
extern void ledflash_enter(Ledflash* handle);

/*! Deactivates the state machine */
extern void ledflash_exit(Ledflash* handle);

/*! Performs a 'run to completion' step. */
extern void ledflash_runCycle(Ledflash* handle);

/*! Raises a time event. */
extern void ledflash_raiseTimeEvent(Ledflash* handle, sc_eventid evid);

/*! Raises the in event 'turnon' that is defined in the default interface scope. */ 
extern void ledflashIface_raise_turnon(Ledflash* handle);

/*! Raises the in event 'turnoff' that is defined in the default interface scope. */ 
extern void ledflashIface_raise_turnoff(Ledflash* handle);


/*!
 * Checks whether the state machine is active (until 2.4.1 this method was used for states).
 * A state machine is active if it was entered. It is inactive if it has not been entered at all or if it has been exited.
 */
extern sc_boolean ledflash_isActive(const Ledflash* handle);

/*!
 * Checks if all active states are final. 
 * If there are no active states then the state machine is considered being inactive. In this case this method returns false.
 */
extern sc_boolean ledflash_isFinal(const Ledflash* handle);

/*! Checks if the specified state is active (until 2.4.1 the used method for states was called isActive()). */
extern sc_boolean ledflash_isStateActive(const Ledflash* handle, LedflashStates state);



#ifdef __cplusplus
}
#endif 

#endif /* LEDFLASH_H_ */
