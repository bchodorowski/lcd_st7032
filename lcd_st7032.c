#include <util/delay.h>

#include "i2cmaster.h"
#include "lcd_st7032.h"

#define LCD_ADDR			0x7c

#define CONTROL_BYTE			0x00 // followed by command bytes
#define CONTROL_BYTE_CO			0x80 // followed by 1 command byte
#define CONTROL_BYTE_RS			0x40 // after last control byte, followed by DDRAM data byte(s)

#define CLEAR_DISPLAY			0x01 // Clear display

#define RETURN_HOME			0x02 // Cursor home to 00H

#define FUNCTION_SET			0x20 // DL: interface data is 8/4 bits, N: number of line is 2/1 DH: double height font, IS: instr
#define FUNCTION_SET_IS			0x01 // IS: instruction table select
#define FUNCTION_SET_DH			0x04 // DH: double height font
#define FUNCTION_SET_N			0x08 // N: number of line is 2/1
#define FUNCTION_SET_DL			0x10 // DL: interface data is 8/4 bits

#define INTERNAL_OSC_FREQ		0x10 // BS=1:1/4 bias, BS=0:1/5 bias, F2~0: adjust internal OSC frequency for FR frequency.
#define INTERNAL_OSC_FREQ_F0		0x01 // F2~0: adjust internal OSC frequency for FR frequency.
#define INTERNAL_OSC_FREQ_F1		0x02 // F2~0: adjust internal OSC frequency for FR frequency.
#define INTERNAL_OSC_FREQ_F2		0x04 // F2~0: adjust internal OSC frequency for FR frequency.
#define INTERNAL_OSC_FREQ_BS		0x08 // BS=1:1/4 bias (BS=0:1/5 bias)

#define POWER_ICON_BOST_CONTR		0x50 // Ion: ICON display on/off, Bon: set booster circuit on/off, C5,C4: Contrast set
#define POWER_ICON_BOST_CONTR_Bon	0x04 // Ion: ICON display on/off
#define POWER_ICON_BOST_CONTR_Ion	0x08 // Bon: set booster circuit on/off

#define FOLLOWER_CONTROL		0x60 // Fon: set follower circuit on/off, Rab2~0: select follower amplified ratio.
#define FOLLOWER_CONTROL_Rab0		0x01 // Rab2~0: select follower amplified ratio
#define FOLLOWER_CONTROL_Rab1		0x02 // Rab2~0: select follower amplified ratio
#define FOLLOWER_CONTROL_Rab2		0x04 // Rab2~0: select follower amplified ratio
#define FOLLOWER_CONTROL_Fon		0x08 // Fon: set follower circuit on/off

#define CONTRAST_SET			0x70 // C0-C3: Contrast set 

#define DISPLAY_ON_OFF			0x08 // display on, cursor on, cursor position on
#define DISPLAY_ON_OFF_B		0x01 // cursor position on
#define DISPLAY_ON_OFF_C		0x02 // cursor on
#define DISPLAY_ON_OFF_D		0x04 // display on

#define SET_DDRAM_ADDRESS		0x80 // Set DDRAM address in address counter
#define SET_CGRAM_ADDRESS		0x40 //Set CGRAM address in address counter

#define LINE_1_ADDR			0x00
#define LINE_2_ADDR			0x40

#define WRITE_DELAY_US			30 // see data sheet
#define HOME_CLEAR_DELAY_US		1200 // see data sheet

static uint8_t contrast = 0x04;
static uint8_t display;

static void writeInstruction(uint8_t cmd)
{
	i2c_start_wait(LCD_ADDR + I2C_WRITE);
	i2c_write(CONTROL_BYTE);
	i2c_write(cmd);
	i2c_stop();
	_delay_us(WRITE_DELAY_US);
}

static void display_on_off()
{
	writeInstruction(DISPLAY_ON_OFF | display);
}

void lcd_st7032_init_i2c()
{
	i2c_init();
	_delay_ms(50); // guesstimated value, seems to work fine
}

void lcd_st7032_init()
{
	writeInstruction(FUNCTION_SET
		| FUNCTION_SET_DL
		| FUNCTION_SET_N
		| FUNCTION_SET_IS);

	writeInstruction(INTERNAL_OSC_FREQ
		| INTERNAL_OSC_FREQ_BS
		| INTERNAL_OSC_FREQ_F2);

	writeInstruction(POWER_ICON_BOST_CONTR
		| POWER_ICON_BOST_CONTR_Ion);
	
	lcd_st7032_set_contrast(contrast);

	writeInstruction(FOLLOWER_CONTROL
		| FOLLOWER_CONTROL_Fon
		| FOLLOWER_CONTROL_Rab2);
	
	display = DISPLAY_ON_OFF_D;
	display_on_off();

	_delay_us(300);
}

void lcd_st7032_set_contrast(uint8_t value)
{
	if (value > ST7032_CONTRAST_MAX)
		value = ST7032_CONTRAST_MIN;
	else if (value < ST7032_CONTRAST_MIN)
		value = ST7032_CONTRAST_MAX;
	writeInstruction(CONTRAST_SET | (value & 0x0f));
	writeInstruction((value >> 4) | POWER_ICON_BOST_CONTR | POWER_ICON_BOST_CONTR_Bon);
	contrast = value;
}

void lcd_st7032_display_on()
{
	display |= DISPLAY_ON_OFF_D;
	display_on_off();
}

void lcd_st7032_display_off()
{
	display &= ~DISPLAY_ON_OFF_D;
	display_on_off();
}

void lcd_st7032_cursor_on()
{
	display |= DISPLAY_ON_OFF_C;
	display_on_off();
}

void lcd_st7032_cursor_off()
{
	display &= ~DISPLAY_ON_OFF_C;
	display_on_off();
}

void lcd_st7032_blink_on()
{
	display |= DISPLAY_ON_OFF_B;
	display_on_off();
}

void lcd_st7032_blink_off()
{
	display &= ~DISPLAY_ON_OFF_B;
	display_on_off();
}

void lcd_st7032_clear()
{
	writeInstruction(CLEAR_DISPLAY);
	_delay_us(HOME_CLEAR_DELAY_US);
}

void lcd_st7032_home()
{
	writeInstruction(RETURN_HOME);
	_delay_us(HOME_CLEAR_DELAY_US);
}

void lcd_st7032_write(uint8_t data)
{
	i2c_start_wait(LCD_ADDR + I2C_WRITE);
	i2c_write(CONTROL_BYTE_RS);
	i2c_write(data);
	i2c_stop();
	_delay_us(WRITE_DELAY_US);
}

void lcd_st7032_print(char* data)
{
	while (*data)
	{
		lcd_st7032_write(*data);
		data++;
	}
}

void lcd_st7032_set_cursor(uint8_t line, uint8_t pos)
{
	uint8_t p;

	if (pos > 15)
		pos = 15;
	if (line == 0)
		p = LINE_1_ADDR + pos;
	else
		p = LINE_2_ADDR + pos;

	writeInstruction(SET_DDRAM_ADDRESS | p);
}

void lcd_st7032_create_char(uint8_t location, uint8_t* charmap)
{
	// change instruction set
	writeInstruction(FUNCTION_SET
		| FUNCTION_SET_DL
		| FUNCTION_SET_N);

	writeInstruction(SET_CGRAM_ADDRESS | ((location & 0x7) << 3));
	for (int i = 0; i < 8; i++)
	{
		lcd_st7032_write(charmap[i]);
	}

	// change back to previous instruction set
	writeInstruction(FUNCTION_SET
		| FUNCTION_SET_DL
		| FUNCTION_SET_N
		| FUNCTION_SET_IS);
}
