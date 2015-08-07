#include "menu.h"
#include "lcd.h"
#include "custom_char.h"
#include "functions.h"

extern int start;


void refresh_menu()
{
	lcd_write_ctrl(LCD_CLEAR);

	lcd_gotoxy(1,0);
	
	lcd_write_data('S');
	lcd_write_data('T');
	lcd_write_data('A');
	lcd_write_data('R');
	lcd_write_data('T');

	lcd_gotoxy(8,0);
	lcd_write_data('S');
	lcd_write_data('E');
	lcd_write_data('T');
	lcd_write_data('T');
	lcd_write_data('I');
	lcd_write_data('N');
	lcd_write_data('G');
	lcd_write_data('S');

	lcd_gotoxy(3,1);
	lcd_write_data('H');
	lcd_write_data('I');
	lcd_write_data('G');
	lcd_write_data('H');
	lcd_write_data(' ');
	lcd_write_data('S');
	lcd_write_data('C');
	lcd_write_data('O');
	lcd_write_data('R');
	lcd_write_data('E');
	lcd_write_data('S');

}

void init_menu(void) {
	//Start randomizer for obstacles
	start_timer_for_randomizer();
	//Add right arrow character
	for(int i=0;i<8;i++)
	{
		lcd_write_ctrl(LCD_CGRAM+i);
		lcd_write_data(__cmenu[i]);
	}

	refresh_menu();

	lcd_gotoxy(0,0);
	lcd_write_data(0);

	start = 1;

}

//Move selection to high scores
void menu_move_scores(){
	
	refresh_menu();

	lcd_gotoxy(2,1);
	lcd_write_data(0);

	start = 3;


}

//Move selection to start
void menu_move_start(){
	
	refresh_menu();

	lcd_gotoxy(0,0);
	lcd_write_data(0);

	start = 1;

}

//Move selection to settings
void menu_move_settings(){
	
	
	refresh_menu();
	lcd_gotoxy(7,0);
	lcd_write_data(0);

	start = 2;

}
