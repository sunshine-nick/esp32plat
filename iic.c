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

#define SDA_PIN  GPIO_NUM_4
#define SCL_PIN  GPIO_NUM_15
#define OLED_RST GPIO_NUM_16

static char tag[] = "i2c ";

////////////////////////////////////////////////////////////////////////////
// External routines
////////////////////////////////////////////////////////////////////////////
void loadctab(char *com, CMD_PFUNC f,char *s);

////////////////////////////////////////////////////////////////////////////
//IIC routines
////////////////////////////////////////////////////////////////////////////
void i2c_master_init()
{
	i2c_config_t i2c_config = {
		.mode = I2C_MODE_MASTER,
		.sda_io_num = SDA_PIN,
		.scl_io_num = SCL_PIN,
		.sda_pullup_en = GPIO_PULLUP_ENABLE,
		.scl_pullup_en = GPIO_PULLUP_ENABLE,
		.master.clk_speed = 1000000
	};
	i2c_param_config(I2C_NUM_0, &i2c_config);
	i2c_driver_install(I2C_NUM_0, I2C_MODE_MASTER, 0, 0, 0);
}

void i2cscanner(void ) {
ESP_LOGI(tag, ">> i2cScanner");
int i;
esp_err_t espRc;
printf("\n");
printf("     0  1  2  3  4  5  6  7  8  9  a  b  c  d  e  f\n");
printf("00:         ");

for (i=3; i< 0x78; i++) {
		i2c_cmd_handle_t cmd = i2c_cmd_link_create();
		i2c_master_start(cmd);
		i2c_master_write_byte(cmd, (i << 1) | I2C_MASTER_WRITE, 1 /* expect ack */);
		i2c_master_stop(cmd);

		espRc = i2c_master_cmd_begin(I2C_NUM_0, cmd, 20/portTICK_PERIOD_MS);
		if (i%16 == 0) {
			printf("\n%.2x:", i);
		}
    switch (espRc)
      {
      case ESP_OK: 			              printf(" %.2x", i); break;
      case ESP_FAIL:  			          printf(" --"); break;
      case ESP_ERR_TIMEOUT:     			printf(" to"); break;
      case ESP_ERR_INVALID_STATE:     printf(" er"); break;
      }
		//ESP_LOGI(tag, "i=%d, rc=%d (0x%x)", i, espRc, espRc);
		i2c_cmd_link_delete(cmd);
	}
	printf("\n");

}


int do_iicscan(void){
i2cscanner();
newline();
return 0;
}

/////////////////////////////////////////////////////////////////////////////
// Initialise IIC module
////////////////////////////////////////////////////////////////////////////
void platform_init_iic(void)
{
loadctab("iics",do_iicscan,"Scan IIC bus");
i2c_master_init();
}