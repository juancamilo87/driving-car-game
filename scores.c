#include <avr/eeprom.h>
#include <stdio.h>
#include <stdlib.h>
#include "custom_char.h"
#include "lcd.h"
#include "functions.h"
#include "scores.h"
#include "menu.h"

extern int menu_scores;
extern int level;
extern int menu;
extern int start;

extern int game_over_sound[7];
extern int end_times[7];

extern int high_score_sound[7];
extern int high_score_times[7];

//Value of char to input
int letter = 65;
//Number of initials written
int number_letter = 0;
//Store initials of user
int letters[3];
//If char has been selected
int written = 0;

void init_scores(void)
{
	//Add left arrow char
	for(int i=0;i<8;i++)
	{
		lcd_write_ctrl(LCD_CGRAM+i);
		lcd_write_data(__csettings[i+16]);
	}


	lcd_write_ctrl(LCD_CLEAR);
	

	char name[3];
	lcd_gotoxy(1,1);
	//Read high score initials
	eeprom_read_block((void*)&name, 0x10, 3);
	//Write initials
	if(name[0]!=0)
	{
		lcd_write_data(name[0]);
	}
	else
	{
		lcd_write_data(' ');
	}
	if(name[1]!=0)
	{
		lcd_write_data(name[1]);
	}
	else
	{
		lcd_write_data(' ');
	}
	if(name[2]!=0)
	{
		lcd_write_data(name[2]);
	}
	else
	{
		lcd_write_data(' ');
	}

	lcd_write_data(' ');
	lcd_write_data('L');

	char slevel[3];
	//Read level of high score
	eeprom_read_block((void*)&slevel, 0x15, 3);
	//print level
	if(slevel[0]!=0)
	{
		lcd_write_data(slevel[0]);
	}
	else
	{
		lcd_write_data(' ');
	}
	if(slevel[1]!=0)
	{
		lcd_write_data(slevel[1]);
	}
	else
	{
		lcd_write_data(' ');
	}
	if(slevel[2]!=0)
	{
		lcd_write_data(slevel[2]);
	}
	else
	{
		lcd_write_data(' ');
	}
	
		
	lcd_gotoxy(0,0);
	lcd_write_data(0);
	lcd_write_data('H');
	lcd_write_data('i');
	lcd_write_data('g');
	lcd_write_data('h');
	lcd_write_data(' ');
	lcd_write_data('S');
	lcd_write_data('c');
	lcd_write_data('o');
	lcd_write_data('r');
	lcd_write_data('e');

	
}

void init_game_over(){
	start_timer_for_randomizer();
	menu = 4;
	menu_scores = 2;
	
	//Add char right arrow
	for(int i=0;i<8;i++)
	{
		lcd_write_ctrl(LCD_CGRAM+i);
		lcd_write_data(__cmenu[i]);
	}
	//Sound of game over
	init_sound_timers(game_over_sound, end_times);

	lcd_write_ctrl(LCD_CLEAR);
	lcd_gotoxy(0,0);
	lcd_write_data('G');
	lcd_write_data('a');
	lcd_write_data('m');
	lcd_write_data('e');
	lcd_write_data(' ');
	lcd_write_data('O');
	lcd_write_data('v');
	lcd_write_data('e');
	lcd_write_data('r');

	lcd_gotoxy(0,1);
	lcd_write_data('L');
	lcd_write_data(' ');

	char slevel[3];
	itoa(level,slevel,10);
	//Clean level string
	if(level<10)
	{
		slevel[1] = ' ';
		slevel[2] = ' ';
	}
	else if(level<100)
	{
		slevel[2] = ' ';
	}
	//print level
	lcd_write_data(slevel[0]);
	lcd_write_data(slevel[1]);
	lcd_write_data(slevel[2]);
	lcd_write_data(' ');

	lcd_gotoxy(9,1);
	lcd_write_data(0);
	lcd_write_data('R');
	lcd_write_data('e');
	lcd_write_data('p');
	lcd_write_data('l');
	lcd_write_data('a');
	lcd_write_data('y');

	lcd_gotoxy(12,0);
	lcd_write_data('M');
	lcd_write_data('e');
	lcd_write_data('n');
	lcd_write_data('u');
}

void init_high_scores(){
	start_timer_for_randomizer();
	//Make sound of high score
	init_sound_timers(high_score_sound, high_score_times);

	menu = 5;
	lcd_write_ctrl(LCD_CLEAR);
	lcd_gotoxy(0,0);
	lcd_write_data('N');
	lcd_write_data('e');
	lcd_write_data('w');
	lcd_write_data(' ');
	lcd_write_data('H');
	lcd_write_data('i');
	lcd_write_data('g');
	lcd_write_data('h');
	lcd_write_data(' ');
	lcd_write_data('S');
	lcd_write_data('c');
	lcd_write_data('o');
	lcd_write_data('r');
	lcd_write_data('e');

	lcd_gotoxy(0,1);

	lcd_write_data('L');
	lcd_write_data(' ');

	char slevel[3];
	itoa(level,slevel,10);
	//Clean level string
	if(level<10)
	{
		slevel[1] = ' ';
		slevel[2] = ' ';
	}
	else if(level<100)
	{
		slevel[2] = ' ';
	}
	//Print level
	lcd_write_data(slevel[0]);
	lcd_write_data(slevel[1]);
	lcd_write_data(slevel[2]);
	lcd_write_data(' ');
	lcd_write_data('N');
	lcd_write_data('a');
	lcd_write_data('m');
	lcd_write_data('e');
	lcd_write_data(':');
	
	//Start blinking
	lcd_gotoxy(11,1);
	lcd_write_ctrl(0x0D);

}

void save_scores(){
	//Stop blinking
	lcd_write_ctrl(LCD_ON);
	char slevel[3];
	itoa(level,slevel,10);
	//Clean level string
	if(level<10)
	{
		slevel[1] = ' ';
		slevel[2] = ' ';
	}
	else if(level<100)
	{
		slevel[2] = ' ';
	}
	//Store on EEPROM level
	eeprom_write_block((void*)&slevel,0x15,3);
	eeprom_write_byte(0x05,level);
	//Store on EEPROM initials
	eeprom_write_byte(0x10,letters[0]);
	eeprom_write_byte(0x11,letters[1]);
	eeprom_write_byte(0x12,letters[2]);
	//Initialize Main menu
	init_menu();
	menu = 1;
	start = 0;
}

//Move selection to main menu (game over menu)
void menu_scores_move_menu(){
	lcd_gotoxy(9,1);
	lcd_write_data(' ');
	lcd_gotoxy(11,0);
	lcd_write_data(0);
	menu_scores = 1;
	
}

//Move selection to replay (game over menu)
void menu_scores_move_replay(){
	lcd_gotoxy(11,0);
	lcd_write_data(' ');
	lcd_gotoxy(9,1);
	lcd_write_data(0);
	menu_scores = 2;
}

//Increase letter (new high score menu)
void increase_letter()
{
	if(number_letter!=3) //All initials not written
	{
		written = 1;
		if(letter >= 90)
		{
			letter = 65;
		}
		else
		{
			letter++;
		}
		lcd_gotoxy(11+number_letter,1);
		lcd_write_data(letter);
		lcd_gotoxy(11+number_letter,1);
	}
	
}

//Decreasae letter (new high score menu)
void decrease_letter(){
	if(number_letter!=3) //All initials not written
	{
		written = 1;
		if(letter <= 65)
		{
			letter = 90;
		}
		else
		{
			letter--;
		}
		lcd_gotoxy(11+number_letter,1);
		lcd_write_data(letter);
		lcd_gotoxy(11+number_letter,1);
	}
}

//Select initial and move to next or store high score
void select_letter()
{
	if(written==1)
	{
		letters[number_letter]=letter;
		letter = 65;
		number_letter++;
		written = 0;
		lcd_gotoxy(11+number_letter,1);
		if(number_letter==3)
		{
			number_letter = 0;
			written = 0;
			letter = 65;
			save_scores();
		}
	}
}
