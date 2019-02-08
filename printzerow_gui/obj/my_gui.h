/* ************************************************************************
      
         my_gui.h
      ================
      Uwe Berger; 2019


---------
Have fun!

************************************************************************* */
#ifndef _MY_GUI_H_
#define _MY_GUI_H_

#include <time.h>
#include <stdio.h>		//printf()
#include <stdlib.h>		//exit()
#include <math.h>

#include "OLED_Driver.h"
#include "OLED_GUI.h"
#include "DEV_Config.h"

#include "my_mqtt.h"

#include "timer.h"

#define DEBOUNCE_MS 50


// ************************************************
// Definitionen fuer Menues
// ************************************************

#define DISPLAY_MAXY	64
#define DISPLAY_MAXX	128
#define MENU_DY 		12		// Zeilenhoehe

typedef struct menu_t menu_t;

struct menu_t
{
	int 	menu_len;
	char 	menu_text[22];
	menu_t 	*submenu;
	void 	*function;
	uint8_t last_pos;
	uint8_t last_view_min;
	uint8_t last_view_max;
};

typedef struct menu_pos_t menu_pos_t;

struct menu_pos_t
{
	menu_t		*menu;
	uint8_t 	pos;
	uint8_t 	view_min;
	uint8_t 	view_max;
	void 		(*function) (void);
	uint8_t 	function_display;
	uint16_t	counter_next_refresh;
	
};


// ************************************************
#define DISPLAY_MENU		0
#define DISPLAY_CLOCK		1
#define DISPLAY_ALL_SENSORS	2
#define DISPLAY_MYWEATHER	3
#define DISPLAY_FORECAST	4
#define DISPLAY_SYSINFO		5

#define TIME_SCREENSAVER_ON 60000    // 60s


// analoge Uhr
#define TWO_PI 6.283185307


// ************************************************
// ************************************************
// ************************************************

void display_menu(void);
void display_clock_digital(void);
void display_clock_analog(void);
void display_all_sensors(void);
void display_myweather(void);
void display_forecast(void);
void display_sysinfo(void);

uint8_t key_bounce();
void key_up(void);
void key_down(void);
void key_left(void);
void key_right(void);
void key_press(void);
void key_1(void);
void key_2(void);
void key_3(void);
void set_button_functions(void);
void display_screen (void);
void display_screensaver(void);



#endif
