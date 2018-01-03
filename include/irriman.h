////////////////////////////////////////////////////////////////////////////
// Sunshine labs
///////////////////////////////////////////////////////////////////////////

///////////////////////////////////////////////////////////////////////////
// RTDB definition
// The Real Time database describes the behavior of the device by holding
// both static configuration and real time dynamic state data. 
// These behavior variables - collectively describe the behavior of a system.
// Irrigation behaviour comprises : valve status, pump status and sensor values
// The collective behaviour values  are different for each product type
//
// Irrigation RTDB structure accessed by FSM
// the RTDB provides all the information about the current irrgation period
// The irrigation  Period FSM contans all the application logic for an irrigation period.
// The irrigation manager FSM has the repsonbibility to update the RTDB for each period.

typedef struct r_tag{
  uint8   devid;      // the device ID as set by the hex switch
  uint8   period;     // The current period
	uint16  valve;		  // status : on/off for valve
	uint16  pump;		    // status : on/off for pump
	uint16 press;		    // pressure of the pump
	uint16 flow;		    // flow if valve is open
	uint16 rtvolume;	  // Run tot vol aggregated each sec valve is open over this period
	uint16 topen;		    // time to open this period
	uint16 tclose;		  // time to close this period
	uint16 rvolume;		  // required volume this period 
	uint16 tcount;		  // actual secs count this period
	uint16 aflow;		    // avg flow this period
	uint16 ptvolume;	  // total volume this period
	uint16 ttvolume;	  // total volume this 24 hours
} rtdb_t;

//////////////////////////////////////////////////////////////////////////////////
// The  iirigation manager uses this information to configure the days work and 
// to configure each period 
//////////////////////////////////////////////////////////////////////////////////
typedef struct w_tag{
	int	topen;
	int	tclose;
  int rqvol;
} wconfig_t;

typedef struct c_tag{
	uint8 dn;				   // device number in chain
  int  notbefore;   // irrigation will not start before this time
  int  notafter;   // irrigtation will not occur after this time
	wconfig_t ttable[4]; // irrigation period configuration
	uint16	rvolume;		 // required volume this 24 hours
} pconfig_t; 

////////////////////////////////////////////////////////////////////////////////////
// the complete chain structure (possibly only used by head of chain)
///////////////////////////////////////////////////////////////////////////////////

typedef struct h_tag{
	uint8 	  chid;			// Chain ID number
	pconfig_t device[10];	// An array of 5 devices in a chain
} chain_t;

//////////////////////////////////////////////////////////////////////////////////////////
//  The data dictionary which is the primary repository for RTDB objects
//////////////////////////////////////////////////////////////////////////////////////////
typedef struct dd_tag
{
char tag[8];
char type;
uint16 *pv;
char units[6];
} dd_t;

extern uint8 gperiod;
extern rtdb_t RTDB;
extern pconfig_t config;
extern uint8 gdevid ;