#include "settings.h"
#include "lcd.h"
#include "custom_char.h"

extern int acceleration;

void init_settings(void){
	//Add costum characters
	for(int i=0;i<24;i++)
	{
		lcd_write_ctrl(LCD_CGRAM+i);
		lcd_write_data(__csettings[i]);
	}

	lcd_write_ctrl(LCD_CLEAR);
	lcd_write_data(2);
	lcd_write_data('E');
	lcd_write_data('n');
	lcd_write_data('a');
	lcd_write_data('b');
	lcd_write_data('l');
	lcd_write_data('e');
	lcd_write_data('/');
	lcd_write_data('D');
	lcd_write_data('i');
	lcd_write_data('s');
	lcd_write_data('a');
	lcd_write_data('b');
	lcd_write_data('l');
	lcd_write_data('e');
	lcd_gotoxy(0,1);
	lcd_write_data('A');
	lcd_write_data('c');
	lcd_write_data('c');
	lcd_write_data('e');
	lcd_write_data('l');
	lcd_write_data('e');
	lcd_write_data('r');
	lcd_write_data('a');
	lcd_write_data('t');
	lcd_write_data('i');
	lcd_write_data('o');
	lcd_write_data('n');

	lcd_gotoxy(15,1);
	if(acceleration == 1)
	{
		lcd_write_data(1);
	}
	else
	{
		lcd_write_data(0);
	}

}


void toggle_acceleration(){
	lcd_gotoxy(15,1);
	if(acceleration == 1)
	{
		
		lcd_write_data(0);
		acceleration = 0;
	}
	else
	{
		lcd_write_data(1);
		acceleration = 1;
	}
}
