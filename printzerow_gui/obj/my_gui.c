/* ************************************************************************
      
         my_gui.c
      ================
      Uwe Berger; 2019


---------
Have fun!

************************************************************************* */

#include "my_gui.h"

char week_day[7][4] = {"Sun", "Mon", "Tue", "Wed", "Thu", "Fri", "Sat"};

menu_t mainmenu[];

// ************************************************
// Vereinbarungen fuer 0.Satz des Arrays:
// * ist kein Menue-Eintrag! 
// * in menu_len steht die Anzahl der Eintraege
//   des entsprechenden Menues (ohne 0.Satz)
// * in menu_text kann ein Menue-Name hinterlegt
//   werden
// * in submenu steht der Verweis auf den 
//   Menue-Vorgaenger
// * in last_pos steht die Cursor-Position, ab
//   der in ein Untermenue verzweigt wurde
// * in last_view_min/max steht der Index der 
//   ersten/letzten angezeigten Zeile, ab der in 
//   ein Untermenue verzweigt wurde
//

menu_t clock_menu[] =
{
	{2, "Clock", 				mainmenu, 	NULL,					0, 0, 0},
	{0, "digital",  			NULL,     	display_clock_digital,	0, 0, 0},
	{0, "analog",   			NULL,     	display_clock_analog,	0, 0, 0}
};

menu_t weather_menu[] =
{
	{3, "Weather",				mainmenu,	NULL,					0, 0, 0},
	{0, "my weather", 			NULL,		display_myweather,		0, 0, 0},
	{0, "my sensors",			NULL,		display_all_sensors,	0, 0, 0},
	{0, "forecast",				NULL,		display_forecast,		0, 0, 0},
	{0, "bla",					NULL,		NULL,		0, 0, 0},
	{0, "blub",					NULL,		NULL,		0, 0, 0}
};

menu_t system_menu[] =
{
	{2, "System", 				mainmenu, 	NULL,					0, 0, 0},
	{0, "reboot",	  			NULL,     	display_reboot,			0, 0, 0},
	{0, "halt",   				NULL,     	display_halt,			0, 0, 0}
};

menu_t mainmenu[] =
{
	{4, "Mainmenu", 			NULL,      		NULL, 				0, 0, 0},
	{0, "Clock",    			clock_menu, 	NULL, 				0, 0, 0},
	{0, "Weather",  			weather_menu, 	NULL, 				0, 0, 0},
	{0, "Sysinfo",   			NULL,      		display_sysinfo,	0, 0, 0},
	{0, "System",	   			system_menu,	NULL,	0, 0, 0},
	{0, "blubb",   				NULL,      		NULL,	0, 0, 0},
	{0, "etc",   				NULL,      		NULL,	0, 0, 0}

};

//menu_pos_t current_menu = {mainmenu, 1, 1, DISPLAY_MAXY/MENU_DY-1, display_menu, 1, 0};
menu_pos_t current_menu = {mainmenu, 1, 1, 3, display_menu, 1, 0};
	

extern sensors_t sensors;
extern myweather_t myweather;
extern forecast_t forecast;
extern sysinfo_t sysinfo;
	
uint8_t current_display = DISPLAY_MENU;

uint8_t screensaver_on = 0;

// ************************************************
void display_menu(void)
{
	uint8_t i;
	uint8_t y = MENU_DY + 4;
	char buf[20];
	char m_pos, s_pos;

	current_display = DISPLAY_MENU;
	OLED_Clear(0x00);
	// Menueueberschrift		
	sprintf(buf, "%s (%i/%i)", current_menu.menu[0].menu_text, current_menu.pos, current_menu.menu[0].menu_len);
	GUI_DisString_EN (1, 0, (const char *)&buf,  &Font12, BLACK, WHITE);
	GUI_DrawLine(0, MENU_DY + 1, DISPLAY_MAXX -1, MENU_DY + 1, WHITE, LINE_SOLID, DOT_PIXEL_1X1);
	// anzuzeigendes Menuefenster berechnen
	if (current_menu.pos > current_menu.view_max) {
		current_menu.view_min++;
		current_menu.view_max++;	
	}
	if (current_menu.view_min > current_menu.pos) {
		current_menu.view_min--;
		current_menu.view_max--;	
	}
	// Menue anzeigen
	for (i=current_menu.view_min; i<=current_menu.view_max; i++) {
		s_pos = ' ';
		if (current_menu.pos == i) {
			m_pos = '>';
			// Submenue vorhanden?
			if (current_menu.menu[i].submenu != NULL) {s_pos = '>';}
		} else {
			m_pos = ' ';
		}
		sprintf(buf, "%c %s %c", m_pos, current_menu.menu[i].menu_text, s_pos);
		GUI_DisString_EN (1, y, (const char *)&buf,  &Font12, FONT_BACKGROUND, WHITE);
		y = y + MENU_DY;
	}
	// ...und raus aufs OLED :-)
	OLED_Display();
	current_menu.function_display = 0;     //
	current_menu.counter_next_refresh = 0; // kein Refresh
}

// ************************************************
void display_clock_digital(void)
{
	time_t now;
    struct tm *timenow;
	char buf[20];

	current_display = DISPLAY_CLOCK;
	time(&now);
	timenow = localtime(&now);
	OLED_Clear(0x00);
	sprintf(buf, "%02i:%02i:%02i", timenow->tm_hour, timenow->tm_min, timenow->tm_sec);
	GUI_DisString_EN (0, 10, (const char *)&buf,  &Font20, FONT_BACKGROUND, WHITE);
	sprintf(buf, "%s, %02i.%02i.%i", &week_day[timenow->tm_wday][0], timenow->tm_mday, timenow->tm_mon+1, timenow->tm_year+1900);
	GUI_DisString_EN (0, 40, (const char *)&buf,  &Font12, FONT_BACKGROUND, WHITE);
	OLED_Display();
	current_menu.function_display = 1;
	current_menu.counter_next_refresh = 200;	// Refresh nach 200ms
}

// ************************************************
void display_clock_analog(void)
{
	time_t now;
    struct tm *timenow;
	char buf[20];
    uint8_t i;
	float x, y, angle;
	const float x0 = 31;
	const float y0 = 31;

	current_display = DISPLAY_CLOCK;
	time(&now);
	timenow = localtime(&now);
	OLED_Clear(0x00);
	// Ziffernblatt
	for (i=0; i<59; i=i+5) {
		angle = TWO_PI*(float)i/60.0;
		y = -30.0 * cos (angle)+0.5;
        x = 30.0 * sin (angle)+0.5;
		GUI_DrawPoint(x+x0, y+y0, WHITE, DOT_PIXEL_1X1, DOT_FILL_AROUND);
	}
	// Stundenzeiger  std=std*5 + dt_m/12
	angle = TWO_PI*((float)timenow->tm_hour*5.0+(float)timenow->tm_min/12.0)/60.0;
	y = -15.0 * cos (angle)+0.5;
    x = 15.0 * sin (angle)+0.5;
	GUI_DrawLine(x0, y0, x0+x, y0+y, WHITE, LINE_SOLID, DOT_PIXEL_2X2);
	// Minutenzeiger
	angle = TWO_PI*(float)timenow->tm_min/60.0;
	y = -29.0 * cos (angle)+0.5;
    x = 29.0 * sin (angle)+0.5;
	GUI_DrawLine(x0, y0, x0+x, y0+y, WHITE, LINE_SOLID, DOT_PIXEL_2X2);
	// Sekundenzeiger
	angle = TWO_PI*(float)timenow->tm_sec/60.0;
	y = -29.0 * cos (angle)+0.5;
    x = 29.0 * sin (angle)+0.5;
	GUI_DrawLine(x0, y0, x0+x, y0+y, WHITE, LINE_SOLID, DOT_PIXEL_1X1);
	sprintf(buf, "%02i:%02i:%02i", timenow->tm_hour, timenow->tm_min, timenow->tm_sec);
	GUI_DisString_EN (70, 10, (const char *)&buf,  &Font12, FONT_BACKGROUND, WHITE);
	GUI_DisString_EN (70, 40, &week_day[timenow->tm_wday][0],  &Font12, FONT_BACKGROUND, WHITE);
	sprintf(buf, "%02i.%02i.%02i", timenow->tm_mday, timenow->tm_mon+1, timenow->tm_year-100);
	GUI_DisString_EN (70, 50, (const char *)&buf,  &Font12, FONT_BACKGROUND, WHITE);
	OLED_Display();
	current_menu.function_display = 1;
	current_menu.counter_next_refresh = 200;	// Refresh nach 200ms
}

// ************************************************
void display_all_sensors(void)
{
	static uint8_t first = 1;
	char buf[20];

	//sensors_t sensors;
	//get_sensors(sensors);
	
	current_display = DISPLAY_ALL_SENSORS;
	OLED_Clear(0x00);	
	// Ueberschrift...
	sprintf(buf, "Sensor %i/%i", (sensors.current_display+1), sensors.count);
	GUI_DisString_EN (1, 0, (const char *)&buf,  &Font12, BLACK, WHITE);
	GUI_DrawLine(0, MENU_DY, DISPLAY_MAXX-1, MENU_DY, WHITE, LINE_SOLID, DOT_PIXEL_1X1);	
	// Daten vorhanden?
	if (sensors.count > 0) {
		// 1.Aufruf und Sensoren vorhanden?
		if (first) {
			first = 0;
			sensors.current_display = 0;	
		}
		// node_alias
		sprintf(buf, "%s", sensors.val[sensors.current_display].node_alias);
		GUI_DisString_EN (1, 16, (const char *)&buf,  &Font12, BLACK, WHITE);
		// temperature
		sprintf(buf, "temp..: %s C", sensors.val[sensors.current_display].temperature);
		GUI_DisString_EN (1, 28, (const char *)&buf,  &Font12, BLACK, WHITE);
		// humidity
		sprintf(buf, "humid.: %s%%", sensors.val[sensors.current_display].humidity);
		GUI_DisString_EN (1, 40, (const char *)&buf,  &Font12, BLACK, WHITE);
		// pressure (nicht immer gesetzt...)
		if (strcmp(sensors.val[sensors.current_display].pressure_rel, "null") != 0) {
			sprintf(buf, "press.: %shPa", sensors.val[sensors.current_display].pressure_rel);
			GUI_DisString_EN (1, 52, (const char *)&buf,  &Font12, BLACK, WHITE);
		}
	}
	OLED_Display();
	current_menu.function_display = 0;
	current_menu.counter_next_refresh = 0;
}

// ************************************************
void display_forecast(void)
{
	static uint8_t first = 1;
	char buf[20];
	
	current_display = DISPLAY_FORECAST;
	OLED_Clear(0x00);	
	// Ueberschrift...
	sprintf(buf, "Forecast %i/%i", (forecast.current_display+1), forecast.count);
	GUI_DisString_EN (1, 0, (const char *)&buf,  &Font12, BLACK, WHITE);
	GUI_DrawLine(0, MENU_DY, DISPLAY_MAXX-1, MENU_DY, WHITE, LINE_SOLID, DOT_PIXEL_1X1);	
	// Daten vorhanden?
	if (forecast.count > 0) {
		// 1.Aufruf und Sensoren vorhanden?
		if (first) {
			first = 0;
			forecast.current_display = 0;	
		}
		sprintf(buf, "%s, %s", forecast.val[forecast.current_display].day, forecast.val[forecast.current_display].date);
		GUI_DisString_EN (1, 16, (const char *)&buf,  &Font12, BLACK, WHITE);
		sprintf(buf, "%s/%s C", forecast.val[forecast.current_display].temp_low, forecast.val[forecast.current_display].temp_high);
		GUI_DisString_EN (1, 28, (const char *)&buf,  &Font12, BLACK, WHITE);
		sprintf(buf, "%s", forecast.val[forecast.current_display].text);
		GUI_DisString_EN (1, 40, (const char *)&buf,  &Font12, BLACK, WHITE);
	}
	OLED_Display();
	current_menu.function_display = 0;
	current_menu.counter_next_refresh = 0;
}

// ************************************************
void display_sysinfo(void)
{
	static uint8_t first = 1;
	char buf[50];
	
	current_display = DISPLAY_SYSINFO;
	OLED_Clear(0x00);	
	// Ueberschrift...
	sprintf(buf, "Sysinfo %i/%i", (sysinfo.current_display+1), sysinfo.count);
	GUI_DisString_EN (1, 0, (const char *)&buf,  &Font12, BLACK, WHITE);
	GUI_DrawLine(0, MENU_DY, DISPLAY_MAXX-1, MENU_DY, WHITE, LINE_SOLID, DOT_PIXEL_1X1);	
	// Daten vorhanden?
	if (sysinfo.count > 0) {
		// 1.Aufruf und Computer vorhanden?
		if (first) {
			first = 0;
			sysinfo.current_display = 0;	
		}
		sprintf(buf, "%s", sysinfo.val[sysinfo.current_display].hostname);
		GUI_DisString_EN (1, 16, (const char *)&buf,  &Font12, BLACK, WHITE);
		switch (sysinfo.current_page) {
			case 0: 
				GUI_DisString_EN (1, 28, "-> uptime:",  &Font12, BLACK, WHITE);
				sprintf(buf, "%s", sysinfo.val[sysinfo.current_display].uptime);
				GUI_DisString_EN (1, 40, (const char *)&buf,  &Font12, BLACK, WHITE);
				GUI_DisString_EN (1, 55, sysinfo.val[sysinfo.current_display].timestamp,  &Font8, BLACK, WHITE);
				break;
			case 1:
				GUI_DisString_EN (1, 28, "-> load:",  &Font12, BLACK, WHITE);
				sprintf(buf, "%s/%s/%s", sysinfo.val[sysinfo.current_display].load_1m, sysinfo.val[sysinfo.current_display].load_5m, sysinfo.val[sysinfo.current_display].load_15m);
				GUI_DisString_EN (1, 40, (const char *)&buf,  &Font12, BLACK, WHITE);
				break;
			case 2:
				GUI_DisString_EN (1, 28, "-> ram:",  &Font12, BLACK, WHITE);
				sprintf(buf, "%s/%s", sysinfo.val[sysinfo.current_display].ram_free, sysinfo.val[sysinfo.current_display].ram_total);
				GUI_DisString_EN (1, 40, (const char *)&buf,  &Font12, BLACK, WHITE);
				sprintf(buf, "%s/%s", sysinfo.val[sysinfo.current_display].ram_share, sysinfo.val[sysinfo.current_display].ram_buffer);
				GUI_DisString_EN (1, 52, (const char *)&buf,  &Font12, BLACK, WHITE);
				break;
			case 3:
				GUI_DisString_EN (1, 28, "-> swap/processes:",  &Font12, BLACK, WHITE);
				sprintf(buf, "%s/%s", sysinfo.val[sysinfo.current_display].swap_free, sysinfo.val[sysinfo.current_display].swap_total);
				GUI_DisString_EN (1, 40, (const char *)&buf,  &Font12, BLACK, WHITE);
				sprintf(buf, "%s", sysinfo.val[sysinfo.current_display].processes);
				GUI_DisString_EN (1, 52, (const char *)&buf,  &Font12, BLACK, WHITE);
				break;
		}

	}
	OLED_Display();
	current_menu.function_display = 0;
	current_menu.counter_next_refresh = 0;
}

// ************************************************
void display_myweather(void)
{
	char buf[20];
	
	current_display = DISPLAY_MYWEATHER;
	OLED_Clear(0x00);	
	// Ueberschrift...
	GUI_DisString_EN (1, 0, "my weather",  &Font12, BLACK, WHITE);
	GUI_DrawLine(0, MENU_DY, DISPLAY_MAXX-1, MENU_DY, WHITE, LINE_SOLID, DOT_PIXEL_1X1);	
	// Werte vorhanden?
	if (myweather.present) {
		sprintf(buf, "temp..:%s C", myweather.temperature);
		GUI_DisString_EN (1, 16, (const char *)&buf,  &Font12, BLACK, WHITE);
		sprintf(buf, "hum...:%s%%", myweather.humidity);
		GUI_DisString_EN (1, 28, (const char *)&buf,  &Font12, BLACK, WHITE);
		sprintf(buf, "press.:%shPa %s", myweather.pressure_rel, myweather.pressure_rising);
		GUI_DisString_EN (1, 40, (const char *)&buf,  &Font12, BLACK, WHITE);
		GUI_DisString_EN (1, 55, myweather.timestamp,  &Font8, BLACK, WHITE);
	}
	OLED_Display();
	current_menu.function_display = 0;
	current_menu.counter_next_refresh = 0;
}

// ************************************************
void display_screensaver(void)
{
	OLED_Clear(0x00);
	OLED_Display();
	screensaver_on = 1;
	// Ausschalten mit key_1, key_2
}

// ************************************************
void screensaver_off(void)
{
	screensaver_on = 0;
	current_menu.function_display = 1;
	current_menu.counter_next_refresh = 0;
}

// ************************************************
void display_reboot(void)
{
	current_display = DISPLAY_REBOOT;
	OLED_Clear(0x00);
	GUI_DisString_EN (1, 0, "Reboot?",  &Font16, BLACK, WHITE);
	GUI_DisString_EN (1, 25, "Cancel --> KEY1",  &Font12, BLACK, WHITE);
	GUI_DisString_EN (1, 37, "Yes    --> KEY3",  &Font12, BLACK, WHITE);
	OLED_Display();
	current_menu.function_display = 0;
	current_menu.counter_next_refresh = 0;
}

// ************************************************
void display_halt(void)
{
	current_display = DISPLAY_HALT;
	OLED_Clear(0x00);
	GUI_DisString_EN (1, 0, "Halt?",  &Font16, BLACK, WHITE);
	GUI_DisString_EN (1, 25, "Cancel --> KEY1",  &Font12, BLACK, WHITE);
	GUI_DisString_EN (1, 37, "Yes    --> KEY3",  &Font12, BLACK, WHITE);
	OLED_Display();
	current_menu.function_display = 0;
	current_menu.counter_next_refresh = 0;
}

// ************************************************
uint8_t key_bounce()
{
	// diese Funktion wird bei jedem Tastendruck aufgerufen, deshalb
	// hier Screensaver-Funktionalitaet einbauen...
	// ...in 60s ohne Taste Display ausschalten
	start_timer(TIME_SCREENSAVER_ON, &display_screensaver);
	// ...falls Display aus, wieder einschalten...
	if (screensaver_on) {
		screensaver_off();
		// "Taste prellt"
		return 1;
	}
	// relativ einfach gestrickt: die Zeit zwischen zwei Aufrufen
	// muss groesser als DEBOUNCE_MS sein:
	// Return: 0 --> Zeit grosser; 1 --> Zeit kleiner (Taste prellt)
	static unsigned ts = 0;
	if (ts < millis()) {
		ts = millis()+DEBOUNCE_MS;
		return 0;
	} else {
		return 1;
	}
}

// ************************************************
void key_up(void) 
{
	if (!key_bounce() && !digitalRead(KEY_UP_PIN)) {
		//puts("key_up");

		switch (current_display) {
			case DISPLAY_MENU:
				if (current_menu.pos > 1) {
					current_menu.pos--;
				}
				break;
			case DISPLAY_SYSINFO:
				if (sysinfo.current_page > 0) {
					sysinfo.current_page--;
				}
				break;
		}
		// Menue anzeigen und Refresh setzen
		current_menu.function_display = 1;
		current_menu.counter_next_refresh = 0;
	}
}


// ************************************************
void key_down(void) 
{
	if (!key_bounce() && !digitalRead(KEY_DOWN_PIN)) {
		//puts("key_down");
	
		switch (current_display) {
			case DISPLAY_MENU:
				if (current_menu.pos < current_menu.menu[0].menu_len) {
					current_menu.pos++;
				}
				break;
			case DISPLAY_SYSINFO:
				if (sysinfo.current_page < sysinfo.pages) {
					sysinfo.current_page++;
				}
				break;
		}

		// Menue anzeigen und Refresh setzen
		current_menu.function_display = 1;
		current_menu.counter_next_refresh = 0;
	}
}

// ************************************************
void key_left(void) 
{
	if (!key_bounce() && !digitalRead(KEY_LEFT_PIN)) {
		//puts("key_left");
		switch (current_display) {
			case DISPLAY_MENU:
				if (current_menu.menu[0].submenu != NULL) {	
					// neue Werte setzen
					current_menu.menu = current_menu.menu[0].submenu;
					current_menu.pos = current_menu.menu[0].last_pos;				
					current_menu.view_min = current_menu.menu[0].last_view_min;				
					current_menu.view_max = current_menu.menu[0].last_view_max;				
				}
				break;
			case DISPLAY_ALL_SENSORS:
				if (sensors.current_display > 0) {
					sensors.current_display--;
				} else {
					sensors.current_display = sensors.count-1;
				}
				break;
			case DISPLAY_FORECAST:
				if (forecast.current_display > 0) {
					forecast.current_display--;
				} else {
					forecast.current_display = forecast.count-1;
				}
				break;
			case DISPLAY_SYSINFO:
				if (sysinfo.current_display > 0) {
					sysinfo.current_display--;
				} else {
					sysinfo.current_display = sysinfo.count-1;
				}
				break;
		}
		// Anzeigen und Refresh setzen
		current_menu.function_display = 1;
		current_menu.counter_next_refresh = 0;
	}
}

// ************************************************
void key_right(void) 
{
	// extern sensors_t sensors;
	
	if (!key_bounce() && !digitalRead(KEY_RIGHT_PIN)) {
		//puts("key_right");
		switch (current_display) {
			case DISPLAY_MENU:
				if (current_menu.menu[current_menu.pos].submenu != NULL) {	
					// pos, view_min, view_max fuer Ruecksprung retten
					current_menu.menu[0].last_pos = current_menu.pos;
					current_menu.menu[0].last_view_min = current_menu.view_min;
					current_menu.menu[0].last_view_max = current_menu.view_max;
					// neue Werte fuer Untermenue setzen
					current_menu.menu = current_menu.menu[current_menu.pos].submenu;
					current_menu.pos = 1;
					current_menu.view_min = 1;
					current_menu.view_max = current_menu.menu[0].menu_len;
					if (current_menu.view_max > DISPLAY_MAXY/MENU_DY-1) {
						current_menu.view_max = DISPLAY_MAXY/MENU_DY-1;
					}
				}
				break;
			case DISPLAY_ALL_SENSORS:
				if (sensors.current_display < (sensors.count-1)) {
					sensors.current_display++;
				} else {
					sensors.current_display = 0;
				}
				break;
			case DISPLAY_FORECAST:
				if (forecast.current_display < (forecast.count-1)) {
					forecast.current_display++;
				} else {
					forecast.current_display = 0;					
				}
				break;
			case DISPLAY_SYSINFO:
				if (sysinfo.current_display < (sysinfo.count-1)) {
					sysinfo.current_display++;
				} else {
					sysinfo.current_display = 0;
				}
				break;
		}
		// Anzeigen und Refresh setzen
		current_menu.function_display = 1;
		current_menu.counter_next_refresh = 0;
	}
}

// ************************************************
void key_press(void) 
{
	if (!key_bounce() && !digitalRead(KEY_PRESS_PIN)) {
		//puts("key_press");

		switch (current_display) {
			case DISPLAY_MENU:
				if (current_menu.menu[current_menu.pos].function != NULL) {
					current_menu.function = current_menu.menu[current_menu.pos].function;
				}
				break;
		}

	}
	// keine Idee, wo sonst rein...:-(
	sysinfo.pages = 3;
	sysinfo.current_page = 0;
	// Refresh Display
	current_menu.function_display = 1;
	current_menu.counter_next_refresh = 0;
}


// ************************************************
void key_1(void) 
{
	if (!key_bounce() && !digitalRead(KEY1_PIN)) {
		//puts("key_1");
		current_menu.function = display_menu;
		// Menue anzeigen und Refresh setzen
		current_menu.function_display = 1;
		current_menu.counter_next_refresh = 0;
	}
}


// ************************************************
void key_2(void) 
{
	if (!key_bounce()) {
		//puts("key_2");
	}
}

// ************************************************
void key_3(void) 
{
	if (!key_bounce() && !digitalRead(KEY3_PIN)) {
		switch (current_display) {
			case DISPLAY_REBOOT:
				system("sudo reboot");
				break;
			case DISPLAY_HALT:
				system("sudo shutdown -Ph now");
				break;
		}
	}
}

// ************************************************
void set_button_functions(void)
{
    wiringPiISR (KEY_UP_PIN, INT_EDGE_BOTH, &key_up);
    wiringPiISR (KEY_DOWN_PIN, INT_EDGE_BOTH, &key_down);
    wiringPiISR (KEY_LEFT_PIN, INT_EDGE_BOTH, &key_left);
    wiringPiISR (KEY_RIGHT_PIN, INT_EDGE_BOTH, &key_right);
    wiringPiISR (KEY_PRESS_PIN, INT_EDGE_BOTH, &key_press);
    wiringPiISR (KEY1_PIN, INT_EDGE_BOTH, &key_1);
    wiringPiISR (KEY2_PIN, INT_EDGE_BOTH, &key_2);
    wiringPiISR (KEY3_PIN, INT_EDGE_BOTH, &key_3);
}

// ************************************************
void display_screen(void)
{
	// Screensaver an?
	if (screensaver_on) return;
	
	
	// ist etwas anzuzeigen?
	if (current_menu.function_display &&        // ja, anzeigen und
		!current_menu.counter_next_refresh)		// Refresh-Zaehler muss 0 sein
	{
		current_menu.function();	
	}
	// ggf. Refresh-Zaehler dekrementieren
	if (current_menu.function_display &&		// ja, anzeigen
		current_menu.counter_next_refresh)		// Refresh-Zaehler noch nicht abgelaufen
	{
		current_menu.counter_next_refresh--;
	}

}
