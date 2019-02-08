/* ************************************************************************
      
           main.c
      ================
      Uwe Berger; 2019


---------
Have fun!

************************************************************************* */
#include <string.h>
#include <unistd.h>


#include "DEV_Config.h"
#include "OLED_Driver.h"
#include "my_gui.h"
#include "my_mqtt.h"
#include "timer.h"


// ****************************************************************************************************************
// ****************************************************************************************************************
// ****************************************************************************************************************
int main(int argc, char **argv)
{

	char mqtt_host[50]	= MQTT_HOST;
	int  mqtt_port    	= MQTT_PORT;
	int c;


	// Aufrufparameter auslesen/verarbeiten
	while ((c=getopt(argc, argv, "h:p:?")) != -1) {
		switch (c) {
			case 'h':
				if (strlen(optarg) >= sizeof(mqtt_host)) {
					puts("hostname too long!");
					exit(EXIT_FAILURE);
				} else {
					strcpy(mqtt_host, optarg);
				}
				break;
			case 'p':
				mqtt_port = atoi(optarg);
				break;
			case '?':
				puts("xxxx [-h <mqtt-host>] [-p <mqtt-port>]");
				puts("xxxx; Uwe Berger, 2018");
				exit(0);
				break;
		}
	}

	// MQTT initialisieren
	if (my_mosquitto_init(mqtt_host, mqtt_port))
		exit(1);

	// RPI-Hardware initialisieren
	if(System_Init()) 
		exit(1);

	// OLED initialisieren
	OLED_Init(SCAN_DIR_DFT);	
	
   	// Buttons jeweils eine Funktion zuweisen
	set_button_functions();

	// Anfangsbildschirm einmal anzeigen
	display_screen();

	// Timer fuer Screensaver starten
	start_timer(TIME_SCREENSAVER_ON, &display_screensaver);

	// Endlos-Loop :-)
	while(1){
		// (eventuell) aktuellen Bildschirm neu anzeigen
		display_screen();
		Driver_Delay_ms(1);							// 1ms Zykluszeit
	}
	
	// Exit
	System_Exit();
	return 0;
}
