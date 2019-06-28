/* ************************************************************************
      
           main.c
      ================
      Uwe Berger; 2019


---------
Have fun!

************************************************************************* */
#include <string.h>
#include <unistd.h>

#include "my_gui.h"
#include "my_mqtt.h"
#include "timer.h"

struct mosquitto *mosq	= NULL;

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
	if (my_mosquitto_init(mqtt_host, mqtt_port)) exit(1);

	// RPI-Hardware initialisieren
	if(System_Init()) exit(1);

	// OLED initialisieren
	OLED_Init(SCAN_DIR_DFT);	
	
   	// Buttons jeweils eine Funktion zuweisen
	set_button_functions();

	// Anfangsbildschirm einmal anzeigen
	display_screen();

	// zyklisch Display-Refresh (ob ueberhaupt wird in Callback-Fkt. entschieden)
	start_timer(SYSTEMTIMER_BEAT, &display_screen);

	// Mosquitto-Endlos-Loop
	mosquitto_loop_forever(mosq, 100000, 1);
	
	// ...der Form halber
	System_Exit();
	return 0;
}
