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

#define    WIFI_CHANNEL_MAX                (13)
#define    WIFI_CHANNEL_SWITCH_INTERVAL    (500)
#define    CONFIG_AP_AUTHMODE   WIFI_AUTH_OPEN
//#define    CONFIG_AP_AUTHMODE   WIFI_AUTH_WPA_WPA2_PSK
//#define    CONFIG_AP_AUTHMODE   WIFI_AUTH_WPA_PSK
//#define    CONFIG_AP_AUTHMODE   WIFI_AUTH_WPA2_PSK

#define    CONFIG_AP_SSID_HIDDEN    0
#define    CONFIG_AP_MAX_CONNECTIONS  4
#define    CONFIG_AP_BEACON_INTERVAL   100

#define    LED_GPIO_PIN       GPIO_NUM_25

typedef struct {
    unsigned frame_ctrl:16;
    unsigned duration_id:16;
    uint8_t addr1[6]; /* receiver address */
    uint8_t addr2[6]; /* sender address */
    uint8_t addr3[6]; /* filtering address */
    unsigned sequence_ctrl:16;
    uint8_t addr4[6]; /* optional */
} wifi_ieee80211_mac_hdr_t;

typedef struct {
    wifi_ieee80211_mac_hdr_t hdr;
    uint8_t payload[0]; /* network data ended with 4 bytes csum (CRC32) */
} wifi_ieee80211_packet_t;

static const char *wifi_sniffer_packet_type2str(wifi_promiscuous_pkt_type_t type);
static void wifi_sniffer_packet_handler(void *buff, wifi_promiscuous_pkt_type_t type);

static const char *TAG = "rtk102";


//#undef ESP_ERROR_CHECK
//#define ESP_ERROR_CHECK(x)   do { esp_err_t rc = (x); if (rc != ESP_OK) { ESP_LOGE("err", "esp_err_t = //%d", rc); assert(0 && #x);} } while(0);

#define core_0 0
#define core_1 1

typedef struct s_tag{
char ssid[20];
char pword[20];
} ssid_t;

ssid_t ssidtab[] = {{"Innovation Dock WiFi","RDMCampus123"},{"Innovation Dock WiFi","RDMCampus123"},{"UPC201428","h2pyznnaOgco"},{"LoRa","" },{"TP-LINK_CCF0","88888888"} };

#define DEFAULT_WIFI_SSID "Guangqiongwifi1"
#define DEFAULT_WIFI_PASS "t35lgq41"
//#define EXAMPLE_WIFI_SSID "TP-LINK_CCF0"
//#define EXAMPLE_WIFI_PASS "88888888"
//#define DEFAULT_WIFI_SSID "Innovation Dock WiFi"
//#define DEFAULT_WIFI_PASS "RDMCampus123"
//#define EXAMPLE_WIFI_SSID "LoRa"
//#define EXAMPLE_WIFI_PASS ""
//#define EXAMPLE_WIFI_SSID "UPC201428"
//#define EXAMPLE_WIFI_PASS "h2pyznnaOgco"

//FreeRTOS event group to signal when we are connected & ready to make a request 
static EventGroupHandle_t wifi_event_group;
//The event group allows multiple bits for each event,
//but we only care about one event - are we connected
//to the AP with an IP? 
const int CONNECTED_BIT = BIT0;
const int SCANDONE_BIT = BIT1;
const int STA_CONNECTED_BIT = BIT2;
const int STA_DISCONNECTED_BIT = BIT3;
// forward declarations
static esp_err_t event_handler(void *ctx, system_event_t *event);
//void wifi_power_save(void);
void get_sntp(void);
//External declarations
void loadctab(char *com, CMD_PFUNC f,char *s);
extern char my_ip[];
/////////

///////////////////////////////////////////////////////////////
// Print station list
////////////////////////////////////////////////////////////////////////
// print the list of connected stations
void printStationList() 
{
	printf(" Connected stations:\n");
	printf("--------------------------------------------------\n");
	
	wifi_sta_list_t wifi_sta_list;
	tcpip_adapter_sta_list_t adapter_sta_list;
   
	memset(&wifi_sta_list, 0, sizeof(wifi_sta_list));
	memset(&adapter_sta_list, 0, sizeof(adapter_sta_list));
   
	ESP_ERROR_CHECK(esp_wifi_ap_get_sta_list(&wifi_sta_list));	
	ESP_ERROR_CHECK(tcpip_adapter_get_sta_list(&wifi_sta_list, &adapter_sta_list));
	
	for(int i = 0; i < adapter_sta_list.num; i++) {
		
		tcpip_adapter_sta_info_t station = adapter_sta_list.sta[i];
         printf("%d - mac: %.2x:%.2x:%.2x:%.2x:%.2x:%.2x - IP: %s\n", i + 1,
				station.mac[0], station.mac[1], station.mac[2],
				station.mac[3], station.mac[4], station.mac[5],
				ip4addr_ntoa(&(station.ip)));
	}
	
	printf("\n");
}
////////////////////////////////////////////////////////////////////////
// WIFI initialisation and event handler
///////////////////////////////////////////////////////////////////////

static esp_err_t event_handler(void *ctx, system_event_t *event)
{
    switch(event->event_id) {
    case SYSTEM_EVENT_SCAN_DONE:
        xEventGroupSetBits(wifi_event_group, SCANDONE_BIT);
        break;        
    case SYSTEM_EVENT_STA_START:
        esp_wifi_connect();
        break;
    case SYSTEM_EVENT_STA_GOT_IP:
        xEventGroupSetBits(wifi_event_group, CONNECTED_BIT);
        ESP_LOGI(TAG, "* - Our IP address is: " IPSTR, IP2STR(&event->event_info.got_ip.ip_info.ip));
        sprintf(my_ip,IPSTR, IP2STR(&event->event_info.got_ip.ip_info.ip));
        break;
    case SYSTEM_EVENT_STA_DISCONNECTED:
        /* This is a workaround as ESP32 WiFi libs don't currently
           auto-reassociate. */
        esp_wifi_connect();
        xEventGroupClearBits(wifi_event_group, CONNECTED_BIT);
        break;
    case SYSTEM_EVENT_AP_START:
		    printf("Access point started\n");
		    break;
    case SYSTEM_EVENT_AP_STACONNECTED:
		    xEventGroupSetBits(wifi_event_group, STA_CONNECTED_BIT);
        printf("Station Joined\n");
        printStationList() ;
	     	break;
  	case SYSTEM_EVENT_AP_STADISCONNECTED:
		    xEventGroupSetBits(wifi_event_group, STA_DISCONNECTED_BIT);
        printf("Station left\n");
        printStationList() ;
		    break;	    
    default:
        break;
    }
    return ESP_OK;
}

//////////////////////////////////////////////////////////////////////
// WIFI commands
//////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////
// sets the global strings for  
// STA ssid and pword
// AP ssid and pword
// and channel number
//////////////////////////////////////////////////////////////////////////
wifi_sta_config_t gsta_config;
wifi_ap_config_t  gap_config;
wifi_config_t gwifi_config;

void wifi_s_config(char *ssid, char *pword)
{
memset(gsta_config.ssid,0,32) ;
memset(gsta_config.password,0,64) ;     
uint8 size = strlen(ssid);
memcpy(gsta_config.ssid,(uint8 *)ssid,size) ;
size = strlen(pword);
memcpy(gsta_config.password,(uint8 *)pword,size) ;
printf("STA: SSID: %s PW: %s \n",gsta_config.ssid,gsta_config.password) ;
}

void wifi_d_config()
{
wifi_config_t wifi_config = {
  .sta = {
    .ssid = DEFAULT_WIFI_SSID,
    .password = DEFAULT_WIFI_PASS,
  },
};
memset(gsta_config.ssid,0,20) ;
memset(gsta_config.password,0,20) ;
memcpy(&gsta_config, &wifi_config,sizeof(wifi_config_t));
printf("STA: SSID: %s PW: %s \n",gsta_config.ssid,gsta_config.password) ;
}

void wifi_a_config(char *apssid, char *apword, uint8 apchan )
{
memset(gap_config.ssid,0,32) ;
memset(gap_config.password,0,64) ;
uint8 size = strlen(apssid); 
memcpy(gap_config.ssid,(uint8 *)apssid,size) ;
size = strlen(apword);
memcpy(gap_config.password,(uint8 *)apword,size) ;

gap_config.ssid_len=0;
gap_config.channel = apchan;
gap_config.authmode = CONFIG_AP_AUTHMODE;
gap_config.ssid_hidden = CONFIG_AP_SSID_HIDDEN;
gap_config.max_connection = CONFIG_AP_MAX_CONNECTIONS;
gap_config.beacon_interval = CONFIG_AP_BEACON_INTERVAL;

printf("AP: SSID: %s PW: %s ", gap_config.ssid, gap_config.password);
printf(" Chan: %d Auth: %d ", gap_config.channel, gap_config.authmode); 
printf(" MaxCon: %d Beacon: %d ms \n", gap_config.max_connection, gap_config.beacon_interval);    
}

////////////////////////////////////////////////////////////////////////
//Initialise and start WIFI in STA or AP or APSTA modes
// config structures should have already been initialised
///////////////////////////////////////////////////////////////////////
void init_wifi(int mode)
{   
wifi_init_config_t wifi_init_config = WIFI_INIT_CONFIG_DEFAULT();
tcpip_adapter_ip_info_t info;
wifi_event_group = xEventGroupCreate();
ESP_ERROR_CHECK( esp_event_loop_init(event_handler, NULL) );
    
switch (mode) {
    case 0: tcpip_adapter_init();

            ESP_ERROR_CHECK(esp_wifi_init(&wifi_init_config));
            ESP_ERROR_CHECK(esp_wifi_set_storage(WIFI_STORAGE_RAM)); 
            ESP_ERROR_CHECK(esp_wifi_set_mode(WIFI_MODE_STA));
            wifi_config_t wifi_config = {
            .sta = {
                    .ssid = DEFAULT_WIFI_SSID,
                    .password = DEFAULT_WIFI_PASS,
                  },
            };
            ESP_ERROR_CHECK(esp_wifi_set_config(ESP_IF_WIFI_STA, &wifi_config));
            ESP_ERROR_CHECK( esp_wifi_start() );
            xEventGroupWaitBits(wifi_event_group, CONNECTED_BIT,false, true, portMAX_DELAY);
            ESP_LOGI(TAG, "Connected to WiFi "); 
            break ;
    case 1: tcpip_adapter_init();
            ESP_ERROR_CHECK(esp_wifi_init(&wifi_init_config));
            ESP_ERROR_CHECK(esp_wifi_set_storage(WIFI_STORAGE_RAM));     
            ESP_ERROR_CHECK(esp_wifi_set_mode(WIFI_MODE_STA));
            memcpy(&gwifi_config,&gsta_config,sizeof(wifi_config_t)) ;
            ESP_ERROR_CHECK(esp_wifi_set_config(ESP_IF_WIFI_STA, &gwifi_config));
            ESP_ERROR_CHECK( esp_wifi_start() );           
            xEventGroupWaitBits(wifi_event_group, CONNECTED_BIT,false, true, portMAX_DELAY);
            ESP_LOGI(TAG, "Connected to WiFi ");                  
            break;
    case 2: tcpip_adapter_init();
            
            ESP_ERROR_CHECK(tcpip_adapter_dhcps_stop(TCPIP_ADAPTER_IF_AP));
            memset(&info, 0, sizeof(info));
            IP4_ADDR(&info.ip, 192, 168, 2, 1);
            IP4_ADDR(&info.gw, 192, 168, 2, 1);
            IP4_ADDR(&info.netmask, 255, 255, 255, 0);
            ESP_ERROR_CHECK(tcpip_adapter_set_ip_info(TCPIP_ADAPTER_IF_AP, &info));
            ESP_ERROR_CHECK(tcpip_adapter_dhcps_start(TCPIP_ADAPTER_IF_AP)) ;
            
            ESP_ERROR_CHECK(esp_wifi_init(&wifi_init_config));
            ESP_ERROR_CHECK(esp_wifi_set_storage(WIFI_STORAGE_RAM));
            ESP_ERROR_CHECK(esp_wifi_set_mode(WIFI_MODE_AP)); 
            memcpy(&gwifi_config,&gap_config,sizeof(wifi_config_t)) ;           
            ESP_ERROR_CHECK(esp_wifi_set_config(WIFI_IF_AP, &gwifi_config));
            
            esp_wifi_start();
            break ;
    case 3: tcpip_adapter_init();
            ESP_ERROR_CHECK(tcpip_adapter_dhcps_stop(TCPIP_ADAPTER_IF_AP));
            memset(&info, 0, sizeof(info));
            IP4_ADDR(&info.ip, 192, 168, 2, 1);
            IP4_ADDR(&info.gw, 192, 168, 2, 1);
            IP4_ADDR(&info.netmask, 255, 255, 255, 0);
            ESP_ERROR_CHECK(tcpip_adapter_set_ip_info(TCPIP_ADAPTER_IF_AP, &info));
            ESP_ERROR_CHECK(tcpip_adapter_dhcps_start(TCPIP_ADAPTER_IF_AP)) ;
            
            ESP_ERROR_CHECK(esp_wifi_init(&wifi_init_config));
            ESP_ERROR_CHECK(esp_wifi_set_storage(WIFI_STORAGE_RAM));
            ESP_ERROR_CHECK( esp_wifi_set_mode(WIFI_MODE_APSTA) ); 
            //Config AP mode
            memcpy(&gwifi_config,&gap_config,sizeof(wifi_config_t)) ;           
            ESP_ERROR_CHECK(esp_wifi_set_config(WIFI_IF_AP, &gwifi_config));
            //Config STA mode
            memcpy(&gwifi_config,&gsta_config,sizeof(wifi_config_t)) ;
            ESP_ERROR_CHECK(esp_wifi_set_config(ESP_IF_WIFI_STA, &gwifi_config));
            esp_wifi_start(); 
            xEventGroupWaitBits(wifi_event_group, CONNECTED_BIT,false, true, portMAX_DELAY);
            ESP_LOGI(TAG, "Connected to WiFi ");    
            break ;
    }
}

void wifi_scan()
{
uint16_t apCount = 0;
int i;

// Let us test a WiFi scan ...
wifi_scan_config_t scanConf = {
      .ssid = NULL,
      .bssid = NULL,
      .channel = 0,
      .show_hidden = true
   };
ESP_ERROR_CHECK(esp_wifi_scan_start(&scanConf, true));    
//The true parameter cause the function to block until done    
xEventGroupWaitBits(wifi_event_group, CONNECTED_BIT,false, true, portMAX_DELAY);                                                                                        
esp_wifi_scan_get_ap_num(&apCount);
if (apCount == 0) return ; 

printf("Number of access points found: %d\n",apCount);
wifi_ap_record_t *list = (wifi_ap_record_t *)malloc(sizeof(wifi_ap_record_t) * apCount);
ESP_ERROR_CHECK(esp_wifi_scan_get_ap_records(&apCount, list));

printf("==========================================================================\n");
printf("CHAN              SSID             |    RSSI    |           AUTH           \n");
printf("==========================================================================\n");
for (i=0; i<apCount; i++) {
         char *authmode;
         switch(list[i].authmode) {
            case WIFI_AUTH_OPEN:
               authmode = "OPEN";
               break;
            case WIFI_AUTH_WEP:
               authmode = "WEP";
               break;           
            case WIFI_AUTH_WPA_PSK:
               authmode = "WPA_PSK";
               break;           
            case WIFI_AUTH_WPA2_PSK:
               authmode = "WPA2_PSK";
               break;           
            case WIFI_AUTH_WPA_WPA2_PSK:
               authmode = "WPA_WPA2_PSK";
               break;
            default:
               authmode = "Unknown";
               break;
         }
         printf("%4d %26.26s    |    % 4d    |    %22.22s\n",list[i].primary,list[i].ssid, list[i].rssi, authmode);
      }
      free(list);
      printf("\n\n");
}

//////////////////////////////////////////////////////////////////////////////
// WIFI sniffer

const char *
wifi_sniffer_packet_type2str(wifi_promiscuous_pkt_type_t type)
{
    switch(type) {
   // case WIFI_PKT_CTRL: return "CTRL";
    case WIFI_PKT_MGMT: return "MGMT";
    case WIFI_PKT_DATA: return "DATA";
    default:    
    case WIFI_PKT_MISC: return "MISC";
    }
}

void
wifi_sniffer_packet_handler(void* buff, wifi_promiscuous_pkt_type_t type)
{

//if (type != WIFI_PKT_MGMT)  return;

const wifi_promiscuous_pkt_t *ppkt = (wifi_promiscuous_pkt_t *)buff;
const wifi_ieee80211_packet_t *ipkt = (wifi_ieee80211_packet_t *)ppkt->payload;
const wifi_ieee80211_mac_hdr_t *hdr = &ipkt->hdr;

printf("PACKET TYPE=%s, CHAN=%02d, RSSI=%02d,"
       " ADDR1=%02x:%02x:%02x:%02x:%02x:%02x,"
       " ADDR2=%02x:%02x:%02x:%02x:%02x:%02x,"
       " ADDR3=%02x:%02x:%02x:%02x:%02x:%02x\n", 
       wifi_sniffer_packet_type2str(type),
       ppkt->rx_ctrl.channel,
       ppkt->rx_ctrl.rssi,
       /* ADDR1 */
       hdr->addr1[0],hdr->addr1[1],hdr->addr1[2],
       hdr->addr1[3],hdr->addr1[4],hdr->addr1[5],
       /* ADDR2 */
       hdr->addr2[0],hdr->addr2[1],hdr->addr2[2],
       hdr->addr2[3],hdr->addr2[4],hdr->addr2[5],
       /* ADDR3 */
       hdr->addr3[0],hdr->addr3[1],hdr->addr3[2],
       hdr->addr3[3],hdr->addr3[4],hdr->addr3[5]
    );
}

void wifi_sniffer(int chan)
{
uint8 level = 1, channel = 0;
channel = chan;
esp_wifi_set_promiscuous(true);
esp_wifi_set_promiscuous_rx_cb(&wifi_sniffer_packet_handler);
ESP_ERROR_CHECK( esp_wifi_start() );    
gpio_set_direction(LED_GPIO_PIN, GPIO_MODE_OUTPUT);
esp_wifi_set_channel(channel, WIFI_SECOND_CHAN_NONE);
gpio_set_level(LED_GPIO_PIN, level); 
   
while ((getLine() == -1)) {
    if (chan == 0) 
        {
        channel = (channel % WIFI_CHANNEL_MAX) + 1;
        esp_wifi_set_channel(channel, WIFI_SECOND_CHAN_NONE);
        }
    vTaskDelay(WIFI_CHANNEL_SWITCH_INTERVAL / portTICK_PERIOD_MS);        
    gpio_set_level(LED_GPIO_PIN, level ^= 1);
  }
esp_wifi_set_promiscuous(false);
esp_wifi_set_promiscuous_rx_cb(0);
}

//END of WIFI functions
//////////////////////////////////////////////////////////////////////////////
//WIFO CLI commands

int do_wifi(void){
int mode = 0;
if (argc >=2) mode = xatol(argv[1]);
else mode = 0;
init_wifi(mode);
newline();
return 0;
}

int do_wscan(void){
	wifi_scan();
	newline();
	return 0;
}

int do_wscfg(void){
int i;

if (argc >=2){
    i = xatol(argv[1]) ;
    wifi_s_config(ssidtab[i].ssid,ssidtab[i].pword);
    }
else //print default table
    {
    for (i=0;i<4;i++) {
      printf("\n SSID : %s PWORD: %s ",ssidtab[i].ssid,ssidtab[i].pword);
      }
    }
newline();
return 0;
}

int do_wacfg(void){
uint8 chan;
if (argc >=4){
  chan = xatol(argv[3]) ;
  wifi_a_config(argv[1],argv[2],chan);
  }
else   wifi_a_config("SS1up","",6); 
   
newline();
return 0;
}

int do_wsniff(void){
int c = 0;
if (argc >=2) c = xatol(argv[1]);
else c = 0;

wifi_sniffer(c);
newline();
return 0;
}

int do_wsli(void){
printStationList();
newline();
return 0;
}

void platform_init_wifi(void)
{

  loadctab("wifi",do_wifi,"Wifi mode [0,1,2,3]");
  loadctab("wscfg",do_wscfg,"STA [n][none]");
  loadctab("wacfg",do_wacfg,"AP [none][ssid pword chan");
  loadctab("wsca",do_wscan,"Make WIFI scan");  
  loadctab("wsni",do_wsniff,"Start WIFI sniffer");
  loadctab("wsli",do_wsli,"AP station list");
}  