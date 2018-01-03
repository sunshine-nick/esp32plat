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
#include "font8x8_basic.h"
#include "font16x16.h"

#define OLED_RST GPIO_NUM_16

static char tag[] = "oled ";

////////////////////////////////////////////////////////////////////////////
// External routines
////////////////////////////////////////////////////////////////////////////
void loadctab(char *com, CMD_PFUNC f,char *s);


//////////////////////////////////////////////////////////////////////////////
// IIC Oled routines

void ssd1306_init() {
	esp_err_t espRc;
  //enable HW
  gpio_set_direction(OLED_RST, GPIO_MODE_OUTPUT);
  gpio_set_level(OLED_RST, 1);

	i2c_cmd_handle_t cmd = i2c_cmd_link_create();

	i2c_master_start(cmd);
	i2c_master_write_byte(cmd, (OLED_I2C_ADDRESS << 1) | I2C_MASTER_WRITE, true);
	i2c_master_write_byte(cmd, OLED_CONTROL_BYTE_CMD_STREAM, true);

	i2c_master_write_byte(cmd, OLED_CMD_SET_CHARGE_PUMP, true);
	i2c_master_write_byte(cmd, 0x14, true);

	i2c_master_write_byte(cmd, OLED_CMD_SET_SEGMENT_REMAP, true); // reverse left-right mapping
	i2c_master_write_byte(cmd, OLED_CMD_SET_COM_SCAN_MODE, true); // reverse up-bottom mapping

	i2c_master_write_byte(cmd, OLED_CMD_DISPLAY_ON, true);
	i2c_master_stop(cmd);

	espRc = i2c_master_cmd_begin(I2C_NUM_0, cmd, 10/portTICK_PERIOD_MS);
	if (espRc == ESP_OK) {
		ESP_LOGI(tag, "OLED configured successfully");
	} else {
		ESP_LOGI(tag, "OLED configuration failed. code: 0x%.2X", espRc);
	}
	i2c_cmd_link_delete(cmd);
}

void ssd1306_onoff(uint8 p) {
	//esp_err_t espRc;
  //enable HW
  gpio_set_direction(OLED_RST, GPIO_MODE_OUTPUT);
  gpio_set_level(OLED_RST, 1);
  
	i2c_cmd_handle_t cmd = i2c_cmd_link_create();

	i2c_master_start(cmd);
	i2c_master_write_byte(cmd, (OLED_I2C_ADDRESS << 1) | I2C_MASTER_WRITE, true);
	i2c_master_write_byte(cmd, OLED_CONTROL_BYTE_CMD_STREAM, true);
	if (p==1) i2c_master_write_byte(cmd, OLED_CMD_DISPLAY_ON, true);
  else i2c_master_write_byte(cmd, OLED_CMD_DISPLAY_OFF, true);
	i2c_master_stop(cmd);

	i2c_master_cmd_begin(I2C_NUM_0, cmd, 10/portTICK_PERIOD_MS);
	
	i2c_cmd_link_delete(cmd);
}


void ssd1306_display_pattern(void *ignore) {
	i2c_cmd_handle_t cmd;

	for (uint8_t i = 0; i < 8; i++) {
		cmd = i2c_cmd_link_create();
		i2c_master_start(cmd);
		i2c_master_write_byte(cmd, (OLED_I2C_ADDRESS << 1) | I2C_MASTER_WRITE, true);
		i2c_master_write_byte(cmd, OLED_CONTROL_BYTE_CMD_SINGLE, true);
		i2c_master_write_byte(cmd, 0xB0 | i, true);
		i2c_master_write_byte(cmd, OLED_CONTROL_BYTE_DATA_STREAM, true);
		for (uint8_t j = 0; j < 128; j++) {
			i2c_master_write_byte(cmd, 0xFF >> (j % 8), true);
		}
		i2c_master_stop(cmd);
		i2c_master_cmd_begin(I2C_NUM_0, cmd, 20/portTICK_PERIOD_MS);
		i2c_cmd_link_delete(cmd);
	}

}

void ssd1306_contrast(uint8 contrast) {
i2c_cmd_handle_t cmd;

cmd = i2c_cmd_link_create();
i2c_master_start(cmd);
i2c_master_write_byte(cmd, (OLED_I2C_ADDRESS << 1) | I2C_MASTER_WRITE, true);
i2c_master_write_byte(cmd, OLED_CONTROL_BYTE_CMD_STREAM, true);    // 0
i2c_master_write_byte(cmd, OLED_CMD_SET_CONTRAST, true);           // 81
i2c_master_write_byte(cmd, contrast, true);                        // contrast value 
i2c_master_stop(cmd);
i2c_master_cmd_begin(I2C_NUM_0, cmd, 20/portTICK_PERIOD_MS);
i2c_cmd_link_delete(cmd);
vTaskDelay(1/portTICK_PERIOD_MS);

}

void ssd1306_scroll(void *ignore) {
esp_err_t espRc;

i2c_cmd_handle_t cmd = i2c_cmd_link_create();
i2c_master_start(cmd);

i2c_master_write_byte(cmd, (OLED_I2C_ADDRESS << 1) | I2C_MASTER_WRITE, true);
i2c_master_write_byte(cmd, OLED_CONTROL_BYTE_CMD_STREAM, true);

i2c_master_write_byte(cmd, 0x29, true); // vertical and horizontal scroll (p29)
i2c_master_write_byte(cmd, 0x00, true);
i2c_master_write_byte(cmd, 0x00, true);
i2c_master_write_byte(cmd, 0x07, true);
i2c_master_write_byte(cmd, 0x01, true);
i2c_master_write_byte(cmd, 0x3F, true);

i2c_master_write_byte(cmd, 0xA3, true); // set vertical scroll area (p30)
i2c_master_write_byte(cmd, 0x20, true);
i2c_master_write_byte(cmd, 0x40, true);

i2c_master_write_byte(cmd, 0x2F, true); // activate scroll (p29)

i2c_master_stop(cmd);
espRc = i2c_master_cmd_begin(I2C_NUM_0, cmd, 20/portTICK_PERIOD_MS);

if (espRc == ESP_OK) {
		ESP_LOGI(tag, "Scroll command succeeded");
	} else {
		ESP_LOGI(tag, "Scroll command failed. code: 0x%.2X", espRc);
	}

i2c_cmd_link_delete(cmd);
}

void ssd1306_setpc(uint8_t l, uint8_t c)
{
i2c_cmd_handle_t cmd;
uint8_t  p;
uint8_t  cl;
uint8_t  ch;

cl = c & 0x0F;
ch = c>>4 | 0x10;
p =  l | 0xB0 ;

cmd = i2c_cmd_link_create();
i2c_master_start(cmd);
i2c_master_write_byte(cmd, (OLED_I2C_ADDRESS << 1) | I2C_MASTER_WRITE, true);

i2c_master_write_byte(cmd, OLED_CONTROL_BYTE_CMD_STREAM, true);
i2c_master_write_byte(cmd, cl, true); // reset column
i2c_master_write_byte(cmd, ch, true);
i2c_master_write_byte(cmd, p , true); // reset page

i2c_master_stop(cmd);
i2c_master_cmd_begin(I2C_NUM_0, cmd, 20/portTICK_PERIOD_MS);
i2c_cmd_link_delete(cmd);
}

void ssd1306_writefont(uint8_t *p)
{   

i2c_cmd_handle_t cmd;
cmd = i2c_cmd_link_create();
i2c_master_start(cmd);
i2c_master_write_byte(cmd, (OLED_I2C_ADDRESS << 1) | I2C_MASTER_WRITE, true);
i2c_master_write_byte(cmd, OLED_CONTROL_BYTE_DATA_STREAM, true);
i2c_master_write(cmd, p ,8, true);
i2c_master_stop(cmd);
i2c_master_cmd_begin(I2C_NUM_0, cmd, 20/portTICK_PERIOD_MS);
i2c_cmd_link_delete(cmd);
}

void ssd1306_display_clear(void *ignore) {
	i2c_cmd_handle_t cmd;

	for (uint8_t i = 0; i < 8; i++) {
		cmd = i2c_cmd_link_create();
		i2c_master_start(cmd);
		i2c_master_write_byte(cmd, (OLED_I2C_ADDRESS << 1) | I2C_MASTER_WRITE, true);
		i2c_master_write_byte(cmd, OLED_CONTROL_BYTE_CMD_SINGLE, true);
		i2c_master_write_byte(cmd, 0xB0 | i, true);
		i2c_master_write_byte(cmd, OLED_CONTROL_BYTE_DATA_STREAM, true);
		for (uint8_t j = 0; j < 128; j++) {
			i2c_master_write_byte(cmd, 0, true);
		}
		i2c_master_stop(cmd);
		i2c_master_cmd_begin(I2C_NUM_0, cmd, 20/portTICK_PERIOD_MS);
		i2c_cmd_link_delete(cmd);
	}
}

void ssd1306_display_text(const void *arg_text) {
char *text = (char*)arg_text;
uint8_t text_len = strlen(text);
uint8_t cur_page = 0;

ssd1306_setpc(cur_page,0);

for (uint8_t i = 0; i < text_len; i++) {
		if (text[i] == '\n') {
      ++cur_page;
      ssd1306_setpc(cur_page,0);
		} else {
     ssd1306_writefont((uint8_t *)font8x8_basic_tr[(uint8_t)text[i]]);   
		}
	}
}

//////////////////////////////////////////////////////////////////////////////
// displays text at line col position
/////////////////////////////////////////////////////////////////////////////
void ssd1306_display_at1(uint8 l, uint8 c, char *arg_text) {
char *text = arg_text;
uint8_t text_len = strlen(text);
uint8_t i;
ssd1306_setpc(l,c);
for (i = 0; i < text_len; i++) ssd1306_writefont((uint8_t *)font8x8_basic_tr[(uint8_t)text[i]]); 

}

static void u8x8_upscale_buf(uint8_t *src, uint8_t *dest)
{
  uint8_t i = 4;  
  do 
  {
    *dest++ = *src;
    *dest++ = *src++;
    i--;
  } while( i > 0 );
}

static uint16_t upscale_byte(uint8_t x)
{
uint16_t y = x;
y |= (y << 4);		// x = (x | (x << S[2])) & B[2];
y &= 0x0f0f;
y |= (y << 2);		// x = (x | (x << S[1])) & B[1];
y &= 0x3333;
y |= (y << 1);		// x = (x | (x << S[0])) & B[0];
y &= 0x5555;
 
y |= (y << 1);		// z = x | (y << 1);
return y;
}


void ssd1306_display_at2(uint8 l, uint8 c, char *arg_text) {
char *text = arg_text;
uint8_t text_len = strlen(text);
uint8_t i,j;
uint8_t buf[8];
uint8_t buf1[8];
uint8_t buf2[8];
uint8_t *ft;
uint16_t t;

for (i = 0; i < text_len; i++) 
  {  
  ft = (uint8_t *)font8x8_basic_tr[(uint8_t )text[i]];
  memcpy(buf,ft,8);
  for (j=0;j<8;j++) 
    {
    t = upscale_byte(buf[j]);
    buf1[j] = t >> 8;
    buf2[j] = t & 255;
    }

  ssd1306_setpc(l,c+i*16); 
  u8x8_upscale_buf(buf2, buf); 
  ssd1306_writefont(&buf[0]); 
  
  ssd1306_setpc(l+1,c+i*16);  
  u8x8_upscale_buf(buf1, buf);
  ssd1306_writefont(&buf[0]); 
  
  ssd1306_setpc(l,c+8+i*16);   
  u8x8_upscale_buf(buf2+4, buf);
  ssd1306_writefont(&buf[0]);  

  ssd1306_setpc(l+1,c+8+i*16);   
  u8x8_upscale_buf(buf1+4, buf);    
  ssd1306_writefont(&buf[0]);   
  }
}

uint8 gsfont = 0;

void lcd_sfont(uint8 p)
{
gsfont=p;
}

void ssd1306_display_at(uint8 l, uint8 c, char *text) 
{
if (gsfont == 0) ssd1306_display_at1(l,c,text);
else ssd1306_display_at2(l,c,text); 
}  

void lcd_onoff(uint8 p)
{
ssd1306_onoff(p);
}

void lcd_test(void)
{
ssd1306_display_pattern(0);
}

void lcd_display_text(char *b)
{
ssd1306_display_text(b);
}

void lcd_clear(void)
{
ssd1306_display_clear(0);
}

void lcd_contrast(uint8 c)
{
ssd1306_contrast(c);
}                                                                                                          

void lcd_text_at(uint8 l, uint8 c, char *s)
{
ssd1306_display_at(l,c,s);
}

/////////////////////////////////////////////////////////////////////////////
// oled lcd commands
/////////////////////////////////////////////////////////////////////////////


int do_oled(void){
lcd_test();
newline();
return 0;
}

int do_oclear(void){
lcd_clear();
newline();
return 0;
}

int do_otext(void){
if (argc >=2)  lcd_display_text(argv[1]);
else lcd_display_text("Hello world 1\nHello World 2\nHello World 3\n");

newline();
return 0;
}

int do_ocontrast(void){
uint8 c;
if (argc >=2) c = xatol(argv[1]);
else c = 100;
lcd_contrast(c);
newline();
return 0;
}

int do_textat(void){
if (argc >=4) {
  uint8 l;
  uint8 c;

  l = xatol(argv[1]);
  c = xatol(argv[2]);
  lcd_text_at(l,c,argv[3]);
  }
newline();
return 0;
}

int do_onoff(void){
uint8 c;

if (argc >=2) { 
  c = xatol(argv[1]);
  lcd_onoff(c);
  }
newline();
return 0;
}

int do_sfont(void){

if (argc >=2) {lcd_sfont(1); }
else lcd_sfont(0);  
newline();
return 0;
}

/////////////////////////////////////////////////////////////////////////////
// Initialise Oled module
////////////////////////////////////////////////////////////////////////////
void platform_init_oled(void)
{ 
loadctab("oled",do_oled,"test Oled LCD");     
loadctab("otxt",do_otext,"Display text"); 
loadctab("oclr",do_oclear,"Clear display"); 
loadctab("ocon",do_ocontrast,"Set display contrast"); 
loadctab("otat",do_textat,"Display text at R,C"); 
loadctab("font",do_sfont,"set font 0/1");
loadctab("onof",do_onoff,"Turn display on or off");  
  
ssd1306_init();
lcd_clear();
lcd_text_at(0,0,"ESP32x platform");
}