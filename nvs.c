
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

nvs_handle my_handle;

////////////////////////////////////////////////////////////////////////////
// External routines
////////////////////////////////////////////////////////////////////////////
void loadctab(char *com, CMD_PFUNC f,char *s);
extern void loadsfile(void);
extern void do_restoresfile(void);

int do_setnvs(void){
int v;

if (argc >=4) {

  v = xatol(argv[2]);
  if (strncmp("i8",argv[1],2) == 0) nvs_set_i8(my_handle,argv[1],v);
  if (strncmp("u8",argv[1],2) == 0) nvs_set_u8(my_handle,argv[1],v);
  if (strncmp("i16",argv[1],3) == 0) nvs_set_i16(my_handle,argv[1],v);
  if (strncmp("u16",argv[1],3) == 0) nvs_set_u16(my_handle,argv[1],v);
  nvs_commit(my_handle);
  }        
newline();
return 0;
}

int do_getnvs(void){
int v;

if (argc >=4) {
  v = xatol(argv[2]);
  if (strncmp("i8",argv[1],2) == 0) nvs_set_i8(my_handle,argv[1],v);
  if (strncmp("u8",argv[1],2) == 0) nvs_set_u8(my_handle,argv[1],v);
  if (strncmp("i16",argv[1],3) == 0) nvs_set_i16(my_handle,argv[1],v);
  if (strncmp("u16",argv[1],3) == 0) nvs_set_u16(my_handle,argv[1],v);
  nvs_commit(my_handle);
  }        
newline();
return 0;
}


void platform_init_nvs(void)
{
const esp_partition_t* nvs_partition;
esp_err_t err = nvs_flash_init();
nvs_partition =  esp_partition_find_first(ESP_PARTITION_TYPE_DATA, ESP_PARTITION_SUBTYPE_DATA_NVS, NULL);     
if(!nvs_partition) { printf("FATAL ERROR: No NVS partition found\n"); return;}

err = nvs_open("storage", NVS_READWRITE, &my_handle);
if (err == ESP_OK) 
  {
  do_restoresfile();
  loadctab("seti",do_setnvs,"set nvs item");  
  loadctab("geti",do_setnvs,"get nvs item");  
  }
else printf("FATAL ERROR: Unable to open NVS\n");

}