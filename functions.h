#ifndef _FUNCTIONS
#define _FUNCTIONS

//Maximum possible rows of obstacles
#define MAX_OBSTACLES  15

void start_timer_for_randomizer();
void print(volatile int level,volatile  int obstacles[],volatile  int position[], int crash_value);
int *create_obstacle(volatile int prev_obstacles[]);
int verify_crash(volatile int obstacles[MAX_OBSTACLES], volatile int position[2]);
void init_sound_timers(int sounds[], int sound_times[]);

#endif
