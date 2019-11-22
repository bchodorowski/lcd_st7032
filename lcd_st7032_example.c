#include "i2cmaster.h"
#include "lcd_st7032.h"

const uint8_t playChar[8] = {
	0b00010000,
	0b00011000,
	0b00011100,
	0b00011110,
	0b00011100,
	0b00011000,
	0b00010000,
	0b00000000,
};

const uint8_t stopChar[8] = {
	0b00000000,
	0b00000000,
	0b00011110,
	0b00011110,
	0b00011110,
	0b00011110,
	0b00000000,
	0b00000000,
};

int main()
{
	lcd_st7032_init_i2c();
	lcd_st7032_init();

	lcd_st7032_create_char(0x01, (uint8_t*)playChar);
	lcd_st7032_create_char(0x02, (uint8_t*)stopChar);

	lcd_st7032_set_cursor(0, 0);
	lcd_st7032_print("Hello World!");
	lcd_st7032_set_cursor(1, 0);
	lcd_st7032_print("Play Stop: \x01 \x02");

	for (;;)
	{
	}
}
