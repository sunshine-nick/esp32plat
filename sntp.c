/* LwIP SNTP example

   This example code is in the Public Domain (or CC0 licensed, at your option.)

   Unless required by applicable law or agreed to in writing, this
   software is distributed on an "AS IS" BASIS, WITHOUT WARRANTIES OR
   CONDITIONS OF ANY KIND, either express or implied.
*/
#include "./include/platform4esp.h"
#include <string.h>
#include <time.h>
#include <sys/time.h>
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "freertos/event_groups.h"
#include "esp_system.h"
#include "esp_wifi.h"
#include "esp_event_loop.h"
#include "esp_log.h"
#include "esp_attr.h"
#include "esp_deep_sleep.h"
#include "nvs_flash.h"
#include "lwip/err.h"
#include "apps/sntp/sntp.h"

long glsecs;
int gelsecs;

void get_sntp(uint8 offset, char sign)
{
time_t now = 0;
struct tm timeinfo = { 0 };
time(&now);
localtime_r(&now, &timeinfo);


if (timeinfo.tm_year < (2016 - 1900)) { // time not set
    debug(1) ("\nConnecting to WiFi and getting time over NTP.");
    debug(1) ("\nInitializing SNTP");
    sntp_setoperatingmode(SNTP_OPMODE_POLL);
    sntp_setservername(0, "pool.ntp.org");
    sntp_init();
    // wait for time to be set
    int retry = 0;
    const int retry_count = 10;
    while(timeinfo.tm_year < (2016 - 1900) && ++retry < retry_count) {
        debug(1) ("\nWaiting for system time to be set... (%d/%d)", retry, retry_count);
        vTaskDelay(2000 / portTICK_PERIOD_MS);
        time(&now);
        localtime_r(&now, &timeinfo);
        }
    sntp_stop();    
    }
// Else Time is set in RTC

time(&now);
char strftime_buf[64];
// Set timezone to GMT
char ts[5];
sprintf(ts,"UTC%c%d",sign,offset);
debug(1) ("\nTimeZone= %s ",ts);
setenv("TZ", ts, 1);
tzset();
localtime_r(&now, &timeinfo);
strftime(strftime_buf, sizeof(strftime_buf), "%c", &timeinfo);
debug(1) ("\nThe current date/time in Greenwich is: %s", strftime_buf);

struct timeval tv;


gettimeofday(&tv,NULL);
debug(1) ("\nEpoch time %ld %ld ",tv.tv_sec, tv.tv_usec) ;

}

int get_secs_smn(void)
{
struct timeval t1, t2;
struct tm tms;
time_t now;
int t;

gettimeofday(&t1,NULL);
time(&now);
localtime_r(&now, &tms);

tms.tm_hour = 0;
tms.tm_min = 0;
tms.tm_sec = 0;
t2.tv_sec = mktime(&tms); 
t = (int )(t1.tv_sec - t2.tv_sec);
return t;
}

void print_time()
{
time_t now = 0;
struct tm timeinfo = { 0 };
struct timeval tv;
char strftime_buf[64];

time(&now);
localtime_r(&now, &timeinfo);
strftime(strftime_buf, sizeof(strftime_buf), "%c", &timeinfo);
debug(1) ( "\nThe current date/time here is: %s", strftime_buf);
gettimeofday(&tv,NULL);
debug(1) ("\nEpoch time %ld %ld ",tv.tv_sec, tv.tv_usec) ;
debug(1) ("\nGlobal linux secs: %ld",glsecs);
debug(1) ("\nElapsed secs since Midnight: %d",gelsecs);
}

void gtimemgr(void)
{
time_t now = 0;
int retry = 0;
struct tm timeinfo = { 0 };
time(&now);
localtime_r(&now, &timeinfo);

while(timeinfo.tm_year < (2016 - 1900) ) 
        {
        debug(2) ("\nSystem Error : Time not set: %d", retry);
        vTaskDelay(2000 / portTICK_PERIOD_MS);
        time(&now);
        localtime_r(&now, &timeinfo);
        retry++;
        if (retry == 120)   exec("rset");
        }
debug(2) ("\nTime is Set, syncing secs from midnight");
gelsecs = get_secs_smn();

while (1)
  {
  time(&now);
  glsecs =  now;
  gelsecs++;
  vTaskDelay(1000 / portTICK_PERIOD_MS);
  }
              
}
///////////////////////////////////////////////////////////////////////////////
//    command section
//////////////////////////////////////////////////////////////////////////////
int do_sntp(void){
uint8 i;
char c;

if (argc >=3){
    i = xatol(argv[1]) ;
    c = argv[2][0] ;
    get_sntp(i,c);
    }
else 	get_sntp(0,'-');

newline();
return 0;
}

int do_time(void){
print_time();
newline();
return 0;
}

int do_esp_reset(void){

esp_restart();
return 0;
}

void platform_init_sntp(void)
{
  loadctab("sntp",do_sntp,"get time");
 	loadctab("time",do_time,"Show time"); 
  loadctab("rset",do_esp_reset,"SW reset");
    
  xTaskCreatePinnedToCore(&gtimemgr, "Glsmgr", 1024*4, NULL, configMAX_PRIORITIES - 1, NULL, core_0);     
}