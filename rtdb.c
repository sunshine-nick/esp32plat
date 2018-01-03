///////////////////////////////////////////////////////////////////////////
// basic object rtdb for creating the FSM behavioural
// Sunshine labs
///////////////////////////////////////////////////////////////////////////
#include "./include/platform4esp.h"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "freertos/event_groups.h"
#include "string.h"
#include "esp_system.h"
#include "esp_event.h"
#include "esp_wifi.h"
#include "esp_wifi_types.h"
#include "esp_event_loop.h"
#include "esp_log.h"
#include "esp_attr.h"
#include "esp_spi_flash.h"
#include "driver/gpio.h"
#include "soc/timer_group_struct.h"
#include "driver/periph_ctrl.h"
#include "driver/timer.h"
#include "esp_partition.h"
#include "esp_err.h"
#include "nvs_flash.h"
#include "nvs.h"
#include "lwip/err.h"
#include <lwip/sockets.h>
#include "./include/irriman.h"

////////////////////////////////////////////////////////////////////////////
// External routines
////////////////////////////////////////////////////////////////////////////
void loadctab(char *com, CMD_PFUNC f,char *s);

rtdb_t RTDB;
pconfig_t config;
dd_t dd[20];
int goi=0; 

int set(char *t, uint16 v)
{
int i;
int l;

for (i=0;i<goi;i++) 
	{
     l = strlen(t);
     if (strncmp(dd[i].tag,t,l)==0) { *dd[i].pv = v; return 1;}
	} 
return 0;
}

int get(char *t, uint16 *v)
{
int i;
int l;
for (i=0;i<goi;i++) 
	{
     l = strlen(t);
     if (strncmp(dd[i].tag,t,l)==0) { *v = *dd[i].pv; return 1;}
	} 
return 0;
}

int check(char *t)
{
int i;
int l;
for (i=0;i<goi;i++) 
	{
     l = strlen(t);
     if (strncmp(dd[i].tag,t,l)==0) return 1;
	} 
return 0;
}

int icreate(char *name, char t, uint16 *v, uint16 d, char *u)
{
int l;
strncpy(dd[goi].tag,name,7);
dd[goi].pv = v;
dd[goi].type = t;
*dd[goi].pv = d;
l = strlen(u);
strncpy(dd[goi].units,u,l);
goi++;
return 0;
}

/////////////////////////////////////////////////////////////////////////////////////
// prints/displays the data dictionary array
////////////////////////////////////////////////////////////////////////////////////
void printdd(void)
{
int i;
for (i=0;i<goi;i++) 
	{
	printf("%s\t%c\t%0x\t%d\t%s",dd[i].tag, dd[i].type, (int)dd[i].pv, *dd[i].pv, dd[i].units);

	printf("\n");
	}
}

/////////////////////////////////////////////////////////////////////////////////////
// prints/displays the value of a single data tag. uses the get function
////////////////////////////////////////////////////////////////////////////////////
void printv(char *s)
{
uint16 v;
get(s,&v);
printf("%s\t%d\n",s,v);
}


/* entry point */ 
int do_rtdb(void)
{
uint16 n;

if (argc > 1)
  {
  if (check(argv[1]) == 0) { printf("Not in RTDB\n"); }
  else 
    {
    n = atoi(argv[2]);
    set(argv[1],n) ;
    }
  }
else 
  {   
  printf("\nRTDB data dictionary\n");
  printdd();
  }
return 0;
}

int  do_config(void)
{
uint8 i;
uint16 to = 0;
uint16 tc = 0;
uint16 rv = 0;

if (argc > 1)
  {
  i = atoi(argv[1]);
  to = atoi(argv[2]);
  tc = atoi(argv[3]);
  rv =atoi(argv[4]); 
  config.ttable[i].topen   = to;
  config.ttable[i].tclose  = tc;
  config.ttable[i].rqvol   = rv;
  }

  printf("Config Table\n");
  printf("Device Nr : %d\n", config .dn);
  printf("Not Before: %d\n", config .notbefore);
  printf("Not After : %d\n\n", config .notafter);
  for (i=0;i<4;i++)
    {
    printf("Period    : %d   ",i);
    printf("Open time : %d  ", config.ttable[i].topen);
    printf("Close Time: %d  ", config.ttable[i].tclose);
    printf("Period Vol: %d\n", config.ttable[i].rqvol);  
    }
  printf("\nReq Volume: %d\n", config .rvolume);
  
return 0;
}

int  do_update(void)
{
uint8 p;
int topen ;
int tclose;
int rvol;

if (argc > 1) {  p = atoi(argv[1]);  }
else {debug(1) ("\nrequires parameter 0-3"); return 0;}

set("period",p);
p--;   // periods are 1,2,3,4 but index starts at 0 
     
topen = config.ttable[p].topen;
tclose = config.ttable[p].tclose;
rvol = config.ttable[p].rqvol;
 

set("topen",topen);
set("tclose",tclose);
set("rvol",rvol);

return 0;
}

void platform_init_rtdb()
{
loadctab("rtdb",do_rtdb,"Display RTDB table  ");
loadctab("cfig",do_config,"Display CONFIG table");
loadctab("uper",do_update,"Update irrigation period");

icreate("devid",'i', (uint16 *)&RTDB.devid, 0, "");
icreate("period",'i', (uint16 *)&RTDB.period, 0, "");
icreate("valve",'i', (uint16 *)&RTDB.valve, 0, "");
icreate("pump",'i',  (uint16 *)&RTDB.pump, 0, "");
icreate("press",'i', (uint16 *)&RTDB.press, 0,"bar");
icreate("flow",'i',  (uint16 *)&RTDB.flow, 0,"lpm");
icreate("rtvol",'i', (uint16 *)&RTDB.rvolume, 0,"l");
icreate("topen",'i', (uint16 *)&RTDB.topen, 0,"secs");
icreate("tclose",'i',(uint16 *)&RTDB.tclose, 0,"secs");
icreate("rvol",'i',  (uint16 *)&RTDB.rvolume, 0,"l");
icreate("tcount",'i',(uint16 *)&RTDB.tcount, 0,"");
icreate("aflow",'i', (uint16 *)&RTDB.aflow, 0,"lpm");
icreate("ptvol",'i', (uint16 *)&RTDB.ptvolume, 0,"l");
icreate("ttvol",'i', (uint16 *)&RTDB.ttvolume, 0,"l");

gdevid=1;
set("devid",gdevid);
set("period",0);
set("valve",1);
set("pump",1);
set("press",21);
set("flow",500);
set("aflow",345);
config.notbefore = 5*3600;
config.notafter = 5*3600 + 12*3600;

config.ttable[0].topen  = 7*3600;
config.ttable[0].tclose = 7*3600 + 30*60;
config.ttable[0].rqvol  = 300; 

config.ttable[1].topen  = 9*3600;
config.ttable[1].tclose = 9*3600 + 30*60;
config.ttable[1].rqvol  = 300; 

config.ttable[2].topen  = 11*3600;
config.ttable[2].tclose = 11*3600 + 30*60;
config.ttable[2].rqvol  = 300; 

config.ttable[3].topen  = 13*3600;
config.ttable[3].tclose = 13*3600 + 30*60;
config.ttable[3].rqvol  = 300; 

config .rvolume = 1200;

}

