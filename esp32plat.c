
#include "./include/platform4esp.h"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "esp_system.h"
#include "esp_spi_flash.h"
#include "driver/gpio.h"
#include "driver/i2c.h"
#include "soc/timer_group_struct.h"
#include "driver/periph_ctrl.h"
#include "driver/timer.h"
#include "esp_log.h"
#include "stdio.h"
#include "sdkconfig.h"
#include "string.h"
#include "ssd1366.h"
#include <rom/rtc.h>

/////////////////////////////////////////////////////////////////////////////
//external routines
/////////////////////////////////////////////////////////////////////////////
void platform_init_iic(void);
void platform_init_oled(void);
void platform_init_wifi(void);
void platform_init_nvs(void);
void platform_init_rtdb(void);
void platform_init_sntp(void);
void platform_init_udp(void);
void platform_init_irrimgr(void);

extern void lcd_clear(void);
extern void lcd_sfont(uint8 n);
extern void lcd_text_at(uint8 l,uint8 c, char *b);

/////////////////////////////////////////////////////////////////////////////
// FSM timersection
// self contained 
////////////////////////////////////////////////////////////////////////////

/////////////////////////////////////////////////////////////////////////////
// FSM interface routines
/////////////////////////////////////////////////////////////////////////////



////////////////////////////////////////////////////////////////////////////
// Start the command line monitor as task
////////////////////////////////////////////////////////////////////////////

void monitor0(void *pvParameter)
{
uint8 c = 2;
c = xPortGetCoreID();

 while(1)
    {
    if (getLine() == 1) do_command(c);
    vTaskDelay(20 / portTICK_PERIOD_MS);
    }  
}

int exec(char *s);

//////////////////////////////////////////////////////////////////////////////
// RESET manager
//////////////////////////////////////////////////////////////////////////////
void print_reason(RESET_REASON reason)
{
  switch ( reason)
  {
    case 1 : printf("POWERON RESET");break;          /**<1, Vbat power on reset*/
    case 3 : printf("SW RESET");break;               /**<3, Software reset digital core*/
    case 4 : printf("OWDT RESET");break;             /**<4, Legacy watch dog reset digital core*/
    case 5 : printf("DEEPSLEEP RESET");break;        /**<5, Deep Sleep reset digital core*/
    case 6 : printf("SDIO RESET");break;             /**<6, Reset by SLC module, reset digital core*/
    case 7 : printf("TG0 WDT SYS RESET");break;       /**<7, Timer Group0 Watch dog reset digital core*/
    case 8 : printf("TG1 WDT SYS RESET");break;       /**<8, Timer Group1 Watch dog reset digital core*/
    case 9 : printf("RTC WDT SYS RESET");break;       /**<9, RTC Watch dog Reset digital core*/
    case 10 : printf("INTRUSION RESET");break;       /**<10, Instrusion tested to reset CPU*/
    case 11 : printf("TG WDT CPU RESET");break;       /**<11, Time Group reset CPU*/
    case 12 : printf("SW CPU RESET");break;          /**<12, Software reset CPU*/
    case 13 : printf("RTC WDT CPU RESET");break;      /**<13, RTC Watch dog Reset CPU*/
    case 14 : printf("EXT CPU RESET");break;         /**<14, for APP CPU, reseted by PRO CPU*/
    case 15 : printf("RTC WDT BROWN OUT RESET");break;/**<15, Reset when the vdd voltage is not stable*/
    case 16 : printf("RTC WDT RTC RESET");break;      /**<16, RTC Watch dog reset digital core and rtc module*/
    default : printf("NO_MEANING");
  }
  
}

//////////////////////////////////////////////////////////////////////////////
void disp_start(void)
{
char tb[10];

sprintf(tb,"WAKEUP");
lcd_clear();
lcd_sfont(1);
lcd_text_at(1,0,tb);
}

void esp32plat_main(void)
{
int r0, r1;
r0 = rtc_get_reset_reason(0) ;
printf("\nCPU 0 : ");
print_reason(r0);
r1 = rtc_get_reset_reason(1) ;
printf("    CPU 1 : ");
print_reason(r1);
printf("\n");

platform_init_nvs();
platform_init_iic();
platform_init_oled();
platform_init_wifi();
platform_init_rtdb();
platform_init_sntp();
platform_init_udp();  

xTaskCreatePinnedToCore(&monitor0, "monitor", 1024*4, NULL, configMAX_PRIORITIES - 1, NULL, core_0);

if ((r0 ==1) || (r0 ==12)) //POWER ON RESET or SW RESET so initialise WIFI and time  
  {
  exec("frun");
  esp_log_level_set("wifi", ESP_LOG_ERROR);  
  }
else       // otherwise dont enable WIFI and time is set from RTC
  {
  esp_log_level_set("wifi", ESP_LOG_ERROR);
  disp_start();
  }

vTaskDelay(1000 / portTICK_PERIOD_MS);  
platform_init_irrimgr();
}
