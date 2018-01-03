///////////////////////////////////////////////////////////////////////////
// Simple Irrigation manager
// Sunshine labs
// All non platform SW place here.
// ..Evaluates time significance
// ..Schedules FSM
// ..manages sleeping
// ..implements Hb messaging
// ..implements command message decode
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

uint8 gperiod;
uint8 gpcode;
uint8 gdevid = 0;

////////////////////////////////////////////////////////////////////////////
// External routines
////////////////////////////////////////////////////////////////////////////
extern void loadctab(char *com, CMD_PFUNC f,char *s);
extern void lcd_clear(void);
extern void lcd_sfont(uint8 n);
extern void lcd_text_at(uint8 l,uint8 c, char *b);
extern uint8 check_period(void);

/////////////////////////////////////////////////////////////////////////////////////
// Update period check
// Returns a period code and sets gperiod
/////////////////////////////////////////////////////////////////////////////////////

uint8 check_period(void)
{
gperiod = 0;
if (gelsecs < config.notbefore  ) return 0;
if ((gelsecs > config.notbefore) && (gelsecs < config.ttable[0].topen))  return 1;

if ((gelsecs > config.ttable[0].topen) && (gelsecs < config.ttable[0].tclose))  {gperiod=1; return 2;}
if ((gelsecs > config.ttable[0].tclose) && (gelsecs < config.ttable[1].topen))  return 3;

if ((gelsecs > config.ttable[1].topen) && (gelsecs < config.ttable[1].tclose))  {gperiod=2; return 4;}
if ((gelsecs > config.ttable[1].tclose) && (gelsecs < config.ttable[2].topen))  return 5;

if ((gelsecs > config.ttable[2].topen) && (gelsecs < config.ttable[2].tclose))  {gperiod=3; return 6;}
if ((gelsecs > config.ttable[2].tclose) && (gelsecs < config.ttable[3].topen))  return 7;

if ((gelsecs > config.ttable[3].topen) && (gelsecs < config.ttable[3].tclose))  {gperiod=4; return 8;}
if ((gelsecs > config.ttable[3].tclose) && (gelsecs < config.notafter))  return 9;

if (gelsecs > config.notafter )  return 10;

return 99;
}
  
void disp_els(void)
{
char tb[10];

sprintf(tb,"%d",gelsecs);
lcd_clear();
lcd_sfont(1);
lcd_text_at(1,0,tb);
}

void disp_period(uint8 pcode)
{
char tb[10];

switch (pcode)
  {
  case 0: sprintf(tb,"Before"); break;
  case 1: sprintf(tb,"B->P1"); break;
  case 2: sprintf(tb,"Period 1"); break;
  case 3: sprintf(tb,"P1->P2"); break;
  case 4: sprintf(tb,"Period 2"); break;
  case 5: sprintf(tb,"P2->P3"); break;
  case 6: sprintf(tb,"Period 3"); break;
  case 7: sprintf(tb,"P3->4"); break;
  case 8: sprintf(tb,"Period 4"); break;
  case 9: sprintf(tb,"P4->A"); break; 
  case 10: sprintf(tb,"After"); break; 
 
  case 99:    
  default: sprintf(tb,"! init"); break;
  }
 
lcd_sfont(1);
lcd_text_at(4,0,tb);
}



void irrimgr(void)
{
int count = 0;
uint8 up0=0;
uint8 up1=0;
uint8 up2=0;
uint8 up3=0;
uint8 fsm=0;

char gcs1[20];

while(1) 
  {
  gpcode = check_period();
  disp_els();
  disp_period(gpcode);
  sprintf(gcs1,"uper %d",gperiod); // set command to update period infomation
  // update only happens once per wake up cycle
  switch (gperiod)    //gperiod is set by update period)
  {
  case 0:
  default: 
          count++; 
          // Add smart sleep period calculation here !!   
          sprintf(gcs1,"deep 10000");
          if (count==10) exec(gcs1); 
          //if (count==10) esp_deep_sleep(1000*1000*60);
          break ; 
  case 1: if (up0==0) { exec(gcs1); up0++; };fsm =1; break; 
  case 2: if (up1==0) { exec(gcs1); up1++; }; fsm = 1;break; 
  case 3: if (up2==0) { exec(gcs1); up2++; }; fsm = 1; break; 
  case 4: if (up3==0) { exec(gcs1); up3++; }; fsm = 1; break; 
  }
  //if (fsm ==1) do_run_fsm();
  vTaskDelay(2000 / portTICK_PERIOD_MS);
  }

}

void platform_init_irrimgr(void)
{
//loadctab("send",do_sudp,"Send UDP message");  
//loadctab("hben",do_hb,"Start UDP heart beat");
xTaskCreatePinnedToCore(&irrimgr, "irriman", 1024*4, NULL, configMAX_PRIORITIES - 1, NULL, core_0);
}
