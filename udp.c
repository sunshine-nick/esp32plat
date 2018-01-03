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




#define    UDP_PORT  3001
#define    DEST_PORT 3000
//#define    DEST_IP "255.255.255.255"
#define    DEST_IP "172.31.81.203"

char my_ip[32];
int hbcount=0;

////////////////////////////////////////////////////////////////////////////
// External routines
////////////////////////////////////////////////////////////////////////////
void loadctab(char *com, CMD_PFUNC f,char *s);

/////////////////////////////////////////////////////////////////////////////
// UDP
// Creates an UDP socket (SOCK_DGRAM) with Internet Protocol Family (PF_INET).
// Protocol family and Address family related. 
/////////////////////////////////////////////////////////////////////////////
int send_udp(char *data, int l, int my_port, char *dest_adr, int dest_port)
{
int socket_fd;
struct sockaddr_in sa,da;
int sent_data;

socket_fd = socket(PF_INET, SOCK_DGRAM, 0);
if ( socket_fd < 0 ) {printf("socket call failed"); return(0);}
memset(&sa, 0, sizeof(struct sockaddr_in));
sa.sin_family = AF_INET;
sa.sin_addr.s_addr = inet_addr(my_ip);
sa.sin_port = htons(my_port);

debug(3) ("Host socket created \n");
if (bind(socket_fd, (struct sockaddr *)&sa, sizeof(struct sockaddr_in)) == -1)
    {
    debug(3) ("Bind to Port Number %d ,IP address %s failed\n",my_port,my_ip /*SENDER_IP_ADDR*/);
    close(socket_fd);
    exit(1);
    }
debug(3) ("Bind success: %s %d\n",my_ip, my_port);

memset(&da, 0, sizeof(struct sockaddr_in));
da.sin_family = AF_INET;
da.sin_addr.s_addr = inet_addr(dest_adr);
da.sin_port = htons(dest_port);

l = strlen(data);
//sendto(socket,dataptr,size,flags,to,tolen)    
sent_data = sendto(socket_fd, data,l,0,(struct sockaddr*)&da,sizeof(da));
if(sent_data < 0) {debug(2) ("Send failed\n"); close(socket_fd); return 0; } 
close(socket_fd);
debug(2) ("Send success: %s %d %s %d\n",DEST_IP, DEST_PORT,data,sent_data); 
return sent_data;
}
/////////////////////////////////////////////////////////////////////////////
//send heartbeat function
char ghbmess1[200];

void make_message(void)
{
char s[20];
ghbmess1[0] =0;
ghbmess1[1] =0;
sprintf(s,"{\"id\":%d,",gdevid);
strcat(ghbmess1,s);
sprintf(s,"\"time\":%ld}",glsecs);
strcat(ghbmess1,s);
// "valve":2,
// "Pump":0,
// "press":0,
// "flow":0,
// "rtvol":0,
// "topen":0,
// "tclose":0,
// "rvol":0,
// "tcount":0,
// "aflow":0,
// "ptvol":0,
// "ttvol":0
}

int send_hb(void)
{
int l;
make_message();
l = strlen(ghbmess1);
l = send_udp(ghbmess1,l,UDP_PORT,DEST_IP,DEST_PORT);
hbcount++;
return l;
}

void hb_manager()
{
int l=0;

while (1){
l= send_hb();
if (l==0) printf("Heart beat error \n");
vTaskDelay(5000 / portTICK_PERIOD_MS);
}
}

///////////////////////////////////////////////////////////////////////////////
int do_hb(void){

xTaskCreatePinnedToCore(&hb_manager, "HBmgr", 1024*4, NULL, configMAX_PRIORITIES - 1, NULL, core_0);
return 0;
}

int do_sudp(void){
char *d;
int l;

if (argc >=2) d = argv[1];
else d = "Hello world";
l = strlen(d);
send_udp(d,l,UDP_PORT,DEST_IP,DEST_PORT);
newline();
return 0;
}

void platform_init_udp(void)
{
loadctab("send",do_sudp,"Send UDP message");  
loadctab("hben",do_hb,"Start UDP heart beat");
}