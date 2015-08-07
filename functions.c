#include <avr/io.h>
#include <avr/interrupt.h>
#include <avr/eeprom.h>
#include <util/delay.h>
#include <stdio.h>
#include <stdlib.h>
#include "lcd.h"
#include "functions.h"
#include "scores.h"

extern int current_sound_times[7];
extern int current_sounds[7];
extern int index_sound;
extern int level;
extern int stop;

int new_obstacles[MAX_OBSTACLES];

//Starts the timer to create seed for the obstacles
void start_timer_for_randomizer()
{
	TCCR3B = (1 << CS30); 
	ETIMSK &= ~(1 << OCIE3A); //Timer3 compare interrupt disabled

	ETIMSK |= (1<<TOIE3); // Timer3 Overflow Interrupt Enable
	
}


//Prints current values on screen with the position of the car, the obstacles and the level value
void print(volatile int level, volatile int obstacles[], volatile int position[], int crash_value)
{
	if(crash_value!=2)
	{
		lcd_write_ctrl(LCD_CLEAR);
		int x = 0;
		//Print level row
		lcd_gotoxy(x,1);
		lcd_write_data(0);
		lcd_gotoxy(x,0);
		if(level <= 50){
			lcd_write_data(7);
		}
		else{
			lcd_write_data('+');
		}
	
		x++;


	
		//Print all obstacles
		for(int i = 0; i<MAX_OBSTACLES; i++){
			int current = obstacles[i];
			if(current == 1){
				lcd_gotoxy(x,1);
				lcd_write_data(3);
			}
			else if(current == 2){
				lcd_gotoxy(x,0);
				lcd_write_data(3);
			}
			else if(current == 3){
				lcd_gotoxy(x,0);
				lcd_write_data(3);
				lcd_gotoxy(x,1);
				lcd_write_data(3);
			}
			x++;
		}
	
		int position_x = position[1];
		int position_y = position[0];
	
	
		/*if(crash_value==2){
			if(position_x == 1)
			{
				lcd_gotoxy(position_y+1,1);
			
			}
			else
			{
				lcd_gotoxy(position_y+1,0);
			}		
			lcd_write_data(2);
		}*/
		//Prints car
		if(crash_value==1){
			if(position_x == 2)
			{
				lcd_gotoxy(position_y+1,1);
			}
			else
			{
				lcd_gotoxy(position_y+1,0);
			}

			lcd_write_data(5);
		}
		else if(crash_value == 0)
		{
			if(position_x == 1)
			{
				lcd_gotoxy(position_y+1,1);
				lcd_write_data(6);
			}
			else if(position_x == 2)
			{
				lcd_gotoxy(position_y+1,1);
				lcd_write_data(4);
			}
			else if(position_x  == 3)
			{
				lcd_gotoxy(position_y+1,0);
				lcd_write_data(6);
			}
			else
			{
				lcd_gotoxy(position_y+1,0);
				lcd_write_data(4);
			}
		}
	}


}

//Creates a new obstacle and adds it to the old ones
/* Obstacles description
	0 = No obstacle
	1 = Left obstacle
	2 = Right obstacle
	3 = Two lane obstacle
	*/
int* create_obstacle(volatile int obstacles[])
{
	
	for(int i = 0; i<MAX_OBSTACLES;i++){
		new_obstacles[i] = 0;
	}

	int current_obstacle = obstacles[0];

	int new_rand = (rand() % 4);
	//Algorithm to make game playable
	if(current_obstacle == 0) //If no obstacle, an obstacle must come
	{
		while(new_rand == 0)
		{
			new_rand = (rand() % 3)+1;
		}
	}
	else if(current_obstacle == 1) //If left lane obstacle no right lane obstacle can come
	{
		while(new_rand == 2)
		{
			new_rand = (rand() % 4);
		}
	}
	else if(current_obstacle == 2) //If right lane obstacle no left lane obstacle can come
	{
		while(new_rand == 1)
		{
			new_rand = (rand() % 4);
		}
	}
	else if(current_obstacle == 3) //If obstacle on both lanes, next obstacle must be empty
	{
		new_rand = 0;
	}
	
	new_obstacles[0] = new_rand;

	for(int i = 0; i<MAX_OBSTACLES-1;i++){
		new_obstacles[i+1] = obstacles[i];
	}

	return &new_obstacles;

}





/*
Verifies crash between obstacles and position
Result:
2 = Crash
1 = Jumping box
0 = No intersection
*/
int verify_crash(volatile int obstacles[MAX_OBSTACLES], volatile int position[2]){

	int result = 0;
	if(obstacles[position[0]]==3) //Two lane obstacle
	{
		if(position[1] == 1 || position[1] == 3) //Not jumping
		{
			result = 2;
		}
		else
		{
			result = 1;
		}
	}
	else if(obstacles[position[0]]==1) //Left lane obstacle
	{
		if(position[1] == 1) //Left lane not jumping
		{
			result = 2;
		}
		else if(position[1] == 2) //Left lane jumping
		{
			result = 1;
		}
	}
	else if(obstacles[position[0]]==2) //Right lane obstacle
	{
		if(position[1] == 3) //Right lane not jumping
		{
			result = 2;
		}
		else if(position[1] == 4) //Right lane jumping
		{
			result = 1;
		}
	}

	//Stop game if crash
	if(result == 2)
	{
		stop = 1;
		//Decide if high score or game over
		int high_score = eeprom_read_byte(0x05);
		if(level>high_score||high_score==255)
		{
			init_high_scores();
		}
		else
		{
			init_game_over();
		}

	}
	return result;

}

//Method to start sound given values to create frequencies and length of each one
void init_sound_timers(int sounds[], int sound_times[])
{
	for(int i = 0; i<7;i++)
	{
		current_sound_times[i] = sound_times[i];
		current_sounds[i] = sounds[i];
	}
	//Start timer 0 prescaler 1024
	TCCR0 |= (1 << CS02) | (1 << CS00); 
	
	TCNT0 = 0;

    TCCR0 |=    (1 << WGM01);

    //Enable compare interrupts
    TIMSK |= (1 << OCIE0);
	
	OCR0 = current_sounds[0];
	index_sound++;
	
	//Start timer 2 prescaler 1024
	TCCR2 |= (1 << CS22) | (1 << CS20);

	TCNT2 = 0;
	//Enable overflow interrupts
	TIMSK |= (1 << TOIE2);
	
}
