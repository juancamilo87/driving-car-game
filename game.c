#include <avr/io.h>
#include <avr/interrupt.h>
#include <avr/eeprom.h>
#include <stdio.h>
#include <stdlib.h>
#include "lcd.h"
#include "functions.h"
#include "custom_char.h"
#include "menu.h"
#include "settings.h"
#include "scores.h"

//Main obstacles to print
volatile int main_obstacles[MAX_OBSTACLES];
//Current level of the game
volatile int level;
//Counter to change level
volatile int system_timer_count;

volatile int speed;
//Increase rate of speed
float increase_rate = 0.9;

/*Position of the car
[0]:y axis of the car 0 = top, 14 = bottom
[1]:x axis of the car
	1: left lane
	2: left lane jumping
	3: right lane
	4: right lane jumping
*/
volatile int position[2];
//0 if playing, 1 if game ended
volatile int stop;
//0 if not jumping
volatile int jumping;
/*Determines which part of the system the user is
0: game
1: main menu
2: settings
3: high scores
4: game over
5: new high score
*/
int menu = 1;
/*Determines selection in the menu game over
1: menu
2: replay
*/
int menu_scores = 2;
/*Determines the selection in the main menu
1: start game
2: settings
3: high scores
*/
int start = 1;
//Acceleration- 0: disabled, 1: enabled
int acceleration = 1;
//Initialize randomizer seed
int seed = 0;

//Values for frequencies
int game_over_sound[7] = {0x13,0x13,0x13,0x19,0x19,0x1E,0x1E};
int start_game_sound[7] = {0x1E, 0x1E, 0x1E,0x19,0x19,0x13,0x13};
int high_score_sound[7] = {0x1A, 0x17, 0x16, 0x13, 0x11, 0xF, 0xE};
int new_level_sound[7] = {0x19, 0x19, 0x19,0x19,0x19,0x19,0x19};
//Values for length of each sound frequency
int high_score_times[7] = {3, 3, 3, 3, 3, 3, 3};
int end_times[7] = {3,10,7,10,10,10,10};
int start_times[7] = {3,3,4,5,5,5,5};
int new_level_times[7] = {1, 1, 1,2,1,2,1};

//counter for length of sounds
volatile int sound_overflow= 0;

//Current sound variables
volatile int current_sounds[7];
volatile int current_sound_times[7];
volatile int index_sound = 0;

//Number of seconds for each level
#define INPUT_KEY_TIMEOUT 10  
//Number of rows of obstacles filled when the game starts
#define INITIAL_FILLED 12


void init_obstacles(){

	for(int i = 0; i<MAX_OBSTACLES;i++)
	{
		main_obstacles[i] = 0;
	}

	int current_obstacle = 0;

	for(int i = 0; i<INITIAL_FILLED;i++)
	{
		int new_rand = (rand() % 4);
		//Proper obstacles algorithm like the one in functions.c
		if(current_obstacle == 0)
		{
			while(new_rand == 0)
			{
				new_rand = (rand() % 3)+1;
			}
		}
		else if(current_obstacle == 1)
		{
			while(new_rand == 2)
			{
				new_rand = (rand() % 4);
			}
		}
		else if(current_obstacle == 2)
		{
			while(new_rand == 1)
			{
				new_rand = (rand() % 4);
			}
		}
		else if(current_obstacle == 3)
		{
			new_rand = 0;
		}
		current_obstacle = new_rand;
		main_obstacles[INITIAL_FILLED-1-i] = new_rand;

	}
	

}

//Store temporal custom characters to CGRAM
void add_custom_char()
{
	//Store Level number character
	if(level <= 50){
		for(int i=0;i<8;i++)
		{
			lcd_write_ctrl(LCD_CGRAM+(8*7)+i);
			lcd_write_data(__cnum[i+(level-1)*8]);
		}
	}
	
	//Store improved car characters after certain level
	if(level >= 10){
		//Store car character
		for(int i=0;i<8;i++)
		{
			lcd_write_ctrl(LCD_CGRAM+(8*6)+i);
			lcd_write_data(__ccar[i]);
		}
		//Store Car on top of box character
		for(int i=0;i<8;i++)
		{
			lcd_write_ctrl(LCD_CGRAM+(8*5)+i);
			lcd_write_data(__ccarjumpbox[i]);
		}
		//Store Car jumping character
		for(int i=0;i<8;i++)
		{
			lcd_write_ctrl(LCD_CGRAM+(8*4)+i);
			lcd_write_data(__ccarjumping[i]);
		}

	}
	
}

//Store custom charachters used constantly during the game to CGRAM
void init_characters(){
	//Store L Character
	for(int i=0;i<8;i++)
	{
		lcd_write_ctrl(LCD_CGRAM+i);
		lcd_write_data(__cgeneric[i]);
	}
	//Store Car character
	for(int i=0;i<8;i++)
	{
		lcd_write_ctrl(LCD_CGRAM+(8*6)+i);
		lcd_write_data(__ccar[8+i]);
	}
	//Store Car on top of box character
	for(int i=0;i<8;i++)
	{
		lcd_write_ctrl(LCD_CGRAM+(8*5)+i);
		lcd_write_data(__ccarjumpbox[8+i]);
	}
	//Store Car jumping character
	for(int i=0;i<8;i++)
	{
		lcd_write_ctrl(LCD_CGRAM+(8*4)+i);
		lcd_write_data(__ccarjumping[8+i]);
	}
	
	//Store Box character
	for(int i=0;i<8;i++)
	{
		lcd_write_ctrl(LCD_CGRAM+(8*3)+i);
		lcd_write_data(__cbox[i]);
	}

	//Store Crash character
	for(int i=0;i<8;i++)
	{
		lcd_write_ctrl(LCD_CGRAM+(8*2)+i);
		lcd_write_data(__cbox[8+i]);
	}

	add_custom_char();
}


//Initialize buttons 
void initialize(void) {

	// Enable global interrupts.            
	sei();

	start_timer_for_randomizer();
	DDRE  |=  (1 << PE4) | (1 << PE5);

	PORTE &= ~(1 << PE4);

	PORTE |=  (1 << PE5);   



	//Initialize buttons
	DDRA &= ~(1 << PA0);
	DDRA &= ~(1 << PA1);
	DDRA &= ~(1 << PA2);
	DDRA &= ~(1 << PA3);
	DDRA &= ~(1 << PA4);


	DDRA |= (1 << PA6);



	//Initialize lcd
	lcd_init();


	lcd_write_ctrl(LCD_ON);
	lcd_write_ctrl(LCD_CLEAR);

		

}


void init_game(void) {

	speed = 23437; //1.5 seconds
	stop = 0;
	jumping = 0;
	
	lcd_write_ctrl(LCD_CLEAR);
	//initialize randomizer with random seed
	srand(seed);
	rand();
	
	init_obstacles();
	//Initialize car position on bottom left of screen
	position[0] = MAX_OBSTACLES -1;
	position[1] = 1;
	//Start level
	system_timer_count = 0;
	level = 1;

	init_characters();

	//Timer1 for scrolling screen
	

	//Prescaler 1024
	TCCR1B |= (1 << CS12) | (1 << CS10); 

    TCCR1A &= ~( (1 << WGM11) | (1 << WGM10) );
    TCCR1B |=    (1 << WGM12);
    TCCR1B &=   ~(1 << WGM13);

    //Enable compare interrupts
    TIMSK |= (1 << OCIE1A);

	OCR1AH = (speed >> 8);
	OCR1AL = speed & 0xff;

    
	//Timer3 to increase level
	
	//Prescaler at 1024
	TCCR3B = (1 << CS12) | (1 << CS10); 
	

	TCCR3A &= ~( (1 << WGM11) | (1 << WGM10) );
    TCCR3B |=    (1 << WGM12);
    TCCR3B &=   ~(1 << WGM13);

	// Enable compare interrupts
	ETIMSK |= (1<<OCIE3A); 
	
	//Set interrupt every second
	OCR3AH = 0x3D;
    OCR3AL = 0x09;
	
	//Make sound to start game
	init_sound_timers(start_game_sound, start_times);


}

//Function called after moving the position
void print_after_move()
{
	int crash_value = verify_crash(main_obstacles,position);
	if(crash_value == 2) stop = 1;
	print(level, main_obstacles, position, crash_value);
}

int main(void) 
{
	initialize();
	init_menu();

	//Variables to avoid multiple presses of buttons
	int changed_position_right = 0;
	int changed_position_left = 0;
	int changed_position_jump = 0;
	int changed_position_forward = 0;
	int changed_position_backward = 0;
	int changed_position_select = 0;
	int changed_position_up = 0;
	int changed_position_down = 0;

	while (1) 
	{

		if(menu==1) //In Menu Screen
		{
			//Move to start
			if(PINA & (1<<PA1))
			{
				changed_position_left = 0;
			}
			else
			{
				if(changed_position_left == 0){
					menu_move_start();
					changed_position_left = 1;
				}
			}

			//Move to settings
			if(PINA & (1<<PA3))
			{
				changed_position_right = 0;
			}
			else
			{
				if(changed_position_right == 0){
					menu_move_settings();
					changed_position_right = 1;
				}
			}
	
			//Move to start
			if(PINA & (1<<PA0))
			{
				changed_position_up = 0;
			}
			else
			{
				if(changed_position_up == 0){
					menu_move_start();
					changed_position_up = 1;
				}
			}

			//Move to High Scores
			if(PINA & (1<<PA4))
			{
				changed_position_down = 0;
			}
			else
			{
				if(changed_position_down == 0){
					menu_move_scores();
					changed_position_down = 1;
				}
			}

			//Select button
			if(PINA & (1<<PA2))
			{
				changed_position_select = 0;
			}
			else
			{
				if(changed_position_select == 0){
					if(start == 1) //Start game
					{
						menu = 0;
						init_game();
						changed_position_jump = 1;
					}
					else if(start == 2) //Start settings
					{
						menu = 2;
						init_settings();
					}
					else if(start == 3) //Start High Scores
					{
						menu = 3;
						init_scores();
					
					}
					changed_position_select = 1;
				
				}
			}
		}
		else if(menu==2) //Settings menu
		{
			//Move to Menu
			if(PINA & (1<<PA1))
			{
				changed_position_left = 0;
			}
			else
			{
				if(changed_position_left == 0){
					init_menu();
					menu = 1;
				}
			}

			//Toggle acceleration
			if(PINA & (1<<PA2))
			{
				changed_position_select = 0;
			}
			else
			{
				if(changed_position_select == 0){
					toggle_acceleration();
					changed_position_select = 1;
				}
			}
		}
		else if(menu==3) //High Scores menu
		{
			//Move to Menu
			if(PINA & (1<<PA1))
			{
				changed_position_left = 0;
			}
			else
			{
				if(changed_position_left == 0){
					init_menu();
					menu = 1;
				}
			}
		}
		else if(menu==4) //Game over menu
		{
			//Move to Menu
			if(PINA & (1<<PA0))
			{
				changed_position_up = 0;
			}
			else
			{
				if(changed_position_up == 0){
					menu_scores_move_menu();
					changed_position_up = 1;
				}
			}

			//Move to Replay
			if(PINA & (1<<PA4))
			{
				changed_position_down = 0;
			}
			else
			{
				if(changed_position_down == 0){
					menu_scores_move_replay();
					changed_position_down = 1;
				}
			}

			//Select button
			if(PINA & (1<<PA2))
			{
				changed_position_select = 0;
			}
			else
			{
				if(changed_position_select == 0){
					if(menu_scores ==1) //Start main menu
					{
						init_menu();
						menu = 1;
						start = 0;
					}
					else if(menu_scores ==2) //Start game
					{
						init_game();
						menu = 0;	
					}
					changed_position_select = 1;
				}
			}
		}
		else if(menu==5) //New High Score menu
		{
			//Increase letter
			if(PINA & (1<<PA0))
			{
				changed_position_up = 0;
			}
			else
			{
				if(changed_position_up == 0){
					increase_letter();
					changed_position_up = 1;
				}
			}

			//Decrease letter
			if(PINA & (1<<PA4))
			{
				changed_position_down = 0;
			}
			else
			{
				if(changed_position_down == 0){
					decrease_letter();
					changed_position_down = 1;
				}
			}

			//Select letter
			if(PINA & (1<<PA2))
			{
				changed_position_select = 0;
			}
			else
			{
				if(changed_position_select == 0){
					select_letter();
					changed_position_select = 1;
				}
			}

		}
		else //GAME
		{
			if(stop == 0) //The game is running
			{
				//Move right
				if(PINA & (1<<PA0))
				{
					changed_position_right = 0;
				}
				else
				{
					if(changed_position_right == 0)
					{
						if(position[1] == 2) //Left lane jumping
						{
							changed_position_right = 1;
							position[1] = 4; //Right lane jumping
							print_after_move();
						}
						else if(position[1] == 1) //Left lane
						{
							changed_position_right = 1;
							position[1] = 3; //Right lane
							print_after_move();
						}
				
					}
				}
				//Jump
				if(PINA & (1<<PA2))
				{
					changed_position_jump = 0;
				}
				else
				{
					if(changed_position_jump == 0)
					{
						if(position[1] == 1) //Left lane
						{
							changed_position_jump = 1;
							position[1] = 2; //Left lane jumping
							jumping = 1; //Initialize jump
							print_after_move();
						}
						else if(position[1] == 3) //Right lane
						{
							changed_position_jump = 1;
							position[1] = 4; //Right lane jumping
							jumping = 1; //Initialize jump
							print_after_move();
						}
				
					}
				}
    

				//Move left
				if(PINA & (1<<PA4))
				{
					changed_position_left = 0;
				}
				else
				{
					if(changed_position_left == 0)
					{
						if(position[1] ==4) //Right lane jumping
						{
							changed_position_left = 1;
							position[1] = 2; //Left lane jumping
							print_after_move();
						}
						else if(position[1] == 3) //Right lane
						{
							changed_position_left = 1;
							position[1] = 1; //Left lane
							print_after_move();
						}
					}
				}
		
				//Move forward
				if(PINA & (1<<PA1)||acceleration==0) //if acceleration disabled always true
				{
					changed_position_forward = 0;
				}
				else
				{
					if(changed_position_forward == 0){
						if(position[1] == 1 || position[1] == 3) //Left or right lane
						{
							if(position[0]>0) //Not in the top
							{
								changed_position_forward = 1;
								position[0] = position[0] - 1;
								print_after_move();
							}						
						}
					}
				}

				//Move backward
				if(PINA & (1<<PA3)||acceleration==0) //if acceleration disabled always true
				{
					changed_position_backward = 0;
				}
				else
				{
					if(changed_position_backward == 0){
						if(position[1] == 1 || position[1] == 3) //Left or right lane
						{
							if(position[0] < MAX_OBSTACLES -1) //Not in the bottom
							{
								changed_position_backward = 1;
								position[0] = position[0] + 1;
								print_after_move();
							}
						}
					}
				}
			}
		}
	}
}

//Adjust scrolling timer for new speed
void increase_speed()
{
	
	speed = speed * increase_rate;

	OCR1AH = (speed >> 8);
    OCR1AL = speed & 0xff;
	


}

void reset_speed()
{
	speed = 23437;

	OCR1AH = (speed >> 8);
    OCR1AL = speed & 0xff;
}

//Scroll screen
ISR(TIMER1_COMPA_vect) {

	if(stop ==0) //If game running
	{
		int *p_obs = create_obstacle(main_obstacles);
		for(int i = 0; i<MAX_OBSTACLES;i++)
		{
			main_obstacles[i] = *(p_obs+i);
		}
		//If jumping, count two scrolls and land the car
		if(jumping >=1)
		{
			if(jumping == 2)
			{
				if(position[1] == 2)
				{
					position[1] = 1;
				}
				else if(position[1] == 4)
				{
					position[1] = 3;
				}
				jumping = 0;
			}
			else
			{
				jumping++;
			}
			
		}		

		int crash_value = verify_crash(main_obstacles,position);
		if(crash_value == 2)
		{
			stop = 1;
		}
		print(level, main_obstacles, position, crash_value);
	}
	
	
}

//Count for increase of level
ISR(TIMER3_COMPA_vect) {

	if(stop == 0) //if game running
	{

		system_timer_count++;
	
		if(system_timer_count % INPUT_KEY_TIMEOUT == 0 && system_timer_count >0) {
	    	level++;
			add_custom_char();
			increase_speed();
			//Sound for new level
			init_sound_timers(new_level_sound, new_level_times);
		}
	}
	else
	{
		reset_speed();
	}	
}

//Timer to generate frequency
ISR(TIMER0_COMP_vect) 
{
	PORTE ^= (1 << PE4) | (1 << PE5);
}

//Timer for length of frequency
ISR(TIMER2_OVF_vect) 
{
	sound_overflow++;
	
	if(sound_overflow>=current_sound_times[index_sound-1]) //Lenght of frequency
	{
		if(index_sound>=7) //stop timers
		{
			sound_overflow = 0;
			TCNT2 = 0;
			TCNT0 = 0;
			
			TCCR0 = 0x00;
			TCCR2 = 0x00;
			TIMSK &= ~(1 << OCIE0);
			TIMSK &= ~(1 << TOIE2);
			index_sound = 0;
		}
		else //Change sound frequency
		{
			OCR0 = current_sounds[index_sound];
			index_sound++;
			
		}
		
		sound_overflow = 0;
	}

}

//Timer to generate random seed for randomizer
ISR(TIMER3_OVF_vect)
{
	seed++;
}
