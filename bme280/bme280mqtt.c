/*
************************************************************************
     bme280mqtt.c
   ================
   Uwe Berger; 2019


---------
Have fun!

************************************************************************
*/

#include <stdlib.h>
#include <string.h>
#include <errno.h>
#include <unistd.h>
#include <mosquitto.h>
#include <stdio.h>
#include <time.h>
#include <wiringPiI2C.h>
#include "bme280.h"



// MQTT-Defaults
#define MQTT_HOST 		"dockstar"
#define MQTT_PORT 		1883
#define MQTT_USER 		""
#define MQTT_PWD 		""
#define MQTT_RETAIN		false
#define MQTT_QOS		1
#define MQTT_KEEPALIVE 	120
#define MY_ALTITUDE		0.0

struct mosquitto *mosq = NULL;

// ***********************************************************
int mosquitto_error_handling(int error)
{
	switch(error)
    {
        case MOSQ_ERR_SUCCESS:
			return 0;
            break;
        case MOSQ_ERR_INVAL:
        case MOSQ_ERR_NOMEM:
        case MOSQ_ERR_NO_CONN:
        case MOSQ_ERR_PROTOCOL:
        case MOSQ_ERR_PAYLOAD_SIZE:
		case MOSQ_ERR_CONN_LOST:
		case MOSQ_ERR_NOT_SUPPORTED:
		case MOSQ_ERR_ERRNO:
				fprintf(stderr, "Mosquitto-Error(%i): %s\n", error, mosquitto_strerror(errno));
				exit(EXIT_FAILURE);
				break;
    }
	return 0;
}

// ***********************************************************
// ***********************************************************
// ***********************************************************
int main(int argc, char **argv)
{	
	char hostname[255] 	= "";
	char buf[512] = "";
	char tbuf[50] = "";
	char topic[100] = "";
	struct tm *tmnow;
	time_t tnow;
	int c;
	char mqtt_host[50]	= MQTT_HOST;
	int  mqtt_port    	= MQTT_PORT;
	bool mqtt_retain    = MQTT_RETAIN;
	int  mqtt_qos	    = MQTT_QOS;
	char mqtt_user[50]	= MQTT_USER;
	char mqtt_pwd[50]   = MQTT_PWD;
	char mqtt_id[50]    = "";
	float altitude      = MY_ALTITUDE;


	// (eigenen) Hostname bestimmen
	gethostname(hostname, sizeof(hostname));
	strncpy(mqtt_id, hostname, sizeof(mqtt_id));

	// Aufrufparameter auslesen/verarbeiten
	while ((c=getopt(argc, argv, "h:p:u:P:q:i:a:r?")) != -1) {
		switch (c) {
			case 'h':
				if (strlen(optarg) >= sizeof mqtt_host) {
					puts("hostname too long!");
					exit(EXIT_FAILURE);
				} else {
					strncpy(mqtt_host, optarg, sizeof(mqtt_host));
				}
				break;
			case 'p':
				mqtt_port = atoi(optarg);
				break;
			case 'q':
				mqtt_qos = atoi(optarg);
				if (mqtt_qos < 0) mqtt_qos = 0;
				if (mqtt_qos > 2) mqtt_qos = 2;
				break;
			case 'u':
				if (strlen(optarg) >= sizeof mqtt_user) {
					puts("username too long!");
					exit(EXIT_FAILURE);
				} else {
					strncpy(mqtt_user, optarg, sizeof(mqtt_user));
				}
				break;
			case 'i':
				if (strlen(optarg) >= sizeof mqtt_id) {
					puts("id too long!");
					exit(EXIT_FAILURE);
				} else {
					strncpy(mqtt_id, optarg, sizeof(mqtt_id));
				}
				break;
			case 'P':
				if (strlen(optarg) >= sizeof mqtt_pwd) {
					puts("password too long!");
					exit(EXIT_FAILURE);
				} else {
					strncpy(mqtt_pwd, optarg, sizeof(mqtt_pwd));
				}
				break;
			case 'r':
				mqtt_retain=true;
				break;
			case 'a':
				altitude = atof(optarg);
				break;
			case '?':
				puts("bme280mqtt  [-h <mqtt-host>]  --> MQTT-Broker     (default: localhost)");
				puts("            [-p <mqtt-port>]  --> MQTT-Port       (default: 1883)");
				puts("            [-U <mqtt-user>]  --> MQTT-User       (default: \"\")"); 
				puts("            [-P <mqtt-pwd>]   --> MQTT-Pwd        (default: \"\")");
				puts("            [-q <mqtt-qos>]   --> MQTT-QoS        (default: 0)");
				puts("            [-r]              --> MQTT-Retain     (default: false)");
				puts("            [-i <mqtt-id>]    --> MQTT-Client-ID  (default: tetrisd)");
				puts("            [-a <altitude]    --> sensor altitude (default: 0.0m)");
				puts("            [-?]              --> print this...");
				exit(0);
				break;
		}
	}
	
	// Init Mosquitto-Lib...
	mosquitto_lib_init();

	// einen Mosquitto-Client erzeugen
	mosq = mosquitto_new(mqtt_id, true, NULL);
	if( mosq == NULL )
	{
		switch(errno){
			case ENOMEM:
				fprintf(stderr, "Error: Out of memory.\n");
				break;
			case EINVAL:
				fprintf(stderr, "Error: Invalid id and/or clean_session.\n");
				break;
		}
		mosquitto_lib_cleanup();
		exit(EXIT_FAILURE);
	}
 
	// MQTT-User/Pwd
	mosquitto_error_handling(mosquitto_username_pw_set(mosq, mqtt_user, mqtt_pwd)); 
   
	// mit MQTT-Broker verbinden
	mosquitto_error_handling(mosquitto_connect(mosq, mqtt_host, mqtt_port, MQTT_KEEPALIVE));
  
	// I2C initialisieren
	int fd = wiringPiI2CSetup(BME280_ADDRESS);
	if(fd < 0) {
		printf("Device not found");
		return -1;
	}

	// BME280-Werte auslesen
	bme280_calib_data cal;
	readCalibrationData(fd, &cal);
	wiringPiI2CWriteReg8(fd, 0xf2, 0x01);   // humidity oversampling x 1
	wiringPiI2CWriteReg8(fd, 0xf4, 0x25);   // pressure and temperature oversampling x 1, mode normal
	bme280_raw_data raw;
	getRawData(fd, &raw);
	// ...und berechnen
	int32_t t_fine = getTemperatureCalibration(&cal, raw.temperature);
	float temperature = compensateTemperature(t_fine); // C
	float pressure_abs = compensatePressure(raw.pressure, &cal, t_fine) / 100; // hPa
	float pressure_rel = getPressureRel(pressure_abs, altitude); // hPa
	float humidity = compensateHumidity(raw.humidity, &cal, t_fine);       // %

	// Timestamp ermitteln/formatieren
	time(&tnow);
	tmnow = localtime(&tnow);
	sprintf(tbuf, "%d/%02d/%02d %02d:%02d:%02d", 
					tmnow->tm_year+1900,
					tmnow->tm_mon+1, 
					tmnow->tm_mday,
					tmnow->tm_hour,
					tmnow->tm_min,
					tmnow->tm_sec
			);

	// MQTT-Topic ermitteln
	sprintf(topic, "sensors/%s/json", hostname);	

	// MQTT-Payload zusammensetzen
	sprintf(buf, "{"
					"\"node_type\":\"bme280\", "
					"\"humidity\":\"%.2f\", "
					"\"pressure_rel\":\"%.2f\", "
					"\"pressure_abs\":\"%.2f\", "
					"\"temperature\":\"%.2f\", "
					"\"node_name\":\"%s\", "
					"\"node_alias\":\"%s\", "
					"\"unixtime\":\"%d\", "
					"\"readable_ts\":\"%s\""
					"}",
    humidity, pressure_rel, pressure_abs, temperature, hostname, hostname, (int)time(NULL), tbuf);
    
	// Debugausgabe
	printf("%s\n", buf);

	//MQTT-Publish
	mosquitto_error_handling(mosquitto_publish(mosq, NULL, topic, strlen(buf), buf, mqtt_qos, mqtt_retain));
	usleep(10000);
  
    // MQTT-Verbindung schliessen und aufraeumen...
	mosquitto_disconnect(mosq);
    mosquitto_destroy(mosq);
    mosquitto_lib_cleanup();

    // ...fertig!
    exit(EXIT_SUCCESS);
}
