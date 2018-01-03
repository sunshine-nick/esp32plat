
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
#include "driver/uart.h"
#include "esp_partition.h"
#include "esp_err.h"
#include "nvs_flash.h"
#include "nvs.h"
#include "lwip/err.h"
#include <lwip/sockets.h>

void esp32plat_main(void);

void wusl_manager(void)
{


}

void app_main()
{
uart_set_baudrate(UART_NUM_0, 115200);
io_init();
cli_init();

/* Print chip information */
esp_chip_info_t chip_info;
esp_chip_info(&chip_info);
printf("This is ESP32 chip with %d CPU cores, WiFi%s%s, ",
    chip_info.cores,
    (chip_info.features & CHIP_FEATURE_BT) ? "/BT" : "",
    (chip_info.features & CHIP_FEATURE_BLE) ? "/BLE" : "");
printf("silicon revision %d, ", chip_info.revision);
printf("%dMB %s flash\n", spi_flash_get_chip_size() / (1024 * 1024),
   (chip_info.features & CHIP_FEATURE_EMB_FLASH) ? "embedded" : "external");
fflush(stdout);

esp32plat_main();

}
