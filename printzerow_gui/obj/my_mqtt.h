/* ************************************************************************
      
         my_gui.h
      ================
      Uwe Berger; 2019


---------
Have fun!

************************************************************************* */
#ifndef _MY_MQTT_H_
#define _MY_MQTT_H_

#include <stdio.h>		//printf()
#include <stdlib.h>		//exit()
#include <errno.h>
#include <string.h>
#include <mosquitto.h>
#include <json-c/json.h>


#include "my_gui.h"

// MQTT-Defaults
#define MQTT_HOST 		"localhost"
#define MQTT_PORT 		1883
#define MQTT_KEEPALIVE 	120

/* MQTT-Payloads

sensors/+/json
{
	"heap":"23216","temperature":"30.8",
"humidity":"41.7","unixtime":"1533665227",
"node_name":"esp8266-9982412","node_alias":"Bad",
"node_type":"dht22","readable_ts":"2018/08/07 20:07:07"
}

myweather/all/json 
{
"timestamp":"02.09.2018, 12:59:49",
"temperature_out":"18.7",
"temperature_in":"20.2",
"humidity_in":"77.3",
"humidity_out":"73.0",
"pressure_rel":"1025.2",
"pressure_rising":"0"
}


weatherforecast/all/json 
{
"city":"Brandenburg",
"longitude":"12.56",
"latitude":"52.41",
"current_date":"Sun, 02.09.2018, 12:07",
"sunrise":"06:23",
"sunset":"19:54",
"forecast":[
		{"day":"Sun","date":"02.09.2018","temp_low":"12","temp_high":"20","text":"Showers","code":"11"},
		{"day":"Mon","date":"03.09.2018","temp_low":"15","temp_high":"27","text":"Mostly Cloudy","code":"28"},
		{"day":"Tue","date":"04.09.2018","temp_low":"16","temp_high":"25","text":"Mostly Sunny","code":"34"},
		{"day":"Wed","date":"05.09.2018","temp_low":"13","temp_high":"23","text":"Partly Cloudy","code":"30"},
		{"day":"Thu","date":"06.09.2018","temp_low":"13","temp_high":"24","text":"Rain","code":"12"},
		{"day":"Fri","date":"07.09.2018","temp_low":"13","temp_high":"20","text":"Mostly Cloudy","code":"28"},
		{"day":"Sat","date":"08.09.2018","temp_low":"11","temp_high":"23","text":"Partly Cloudy","code":"30"}
	]
}

zerow/sysinfo/json 
{ 
	"hostname": "zerow", 
	"uptime": "2 days, 2:34:21", 
	"load": [ "0.32", "0.25", "0.27" ], 
	"ram": { "free": "234832", "share": "21912", "buffer": "39472", "total": "492620" }, 
	"swap": { "free": "102396", "total": "102396" }, 
	"processes": "108", 
	"time": { "unix": "1536080941", "readable": "2018\/09\/04 19:09:01" } 
}

*/

#define MAX_SYSINFO			20

typedef struct sysinfo_val_t sysinfo_val_t;
struct sysinfo_val_t
{
	char hostname[21];
	char uptime[20];
	char load_1m[8];
	char load_5m[8];
	char load_15m[8];
	char ram_free[10];
	char ram_share[10];
	char ram_buffer[10];
	char ram_total[10];
	char swap_free[10];
	char swap_total[10];
	char processes[6];
	char timestamp[25];
};

typedef struct sysinfo_t sysinfo_t;
struct sysinfo_t
{
	uint8_t	count;
	uint8_t current_display;
	uint8_t pages;
	uint8_t current_page;
	sysinfo_val_t val[MAX_SYSINFO];
};

#define MAX_FORECASTDAYS	7

typedef struct forecast_val_t forecast_val_t;
struct forecast_val_t
{
	char    day[4];
	char    date[11];
	char    temp_low[5];
	char 	temp_high[5];
	char 	text[50];
	char 	code[3];
};

typedef struct forecast_t forecast_t;
struct forecast_t
{
	uint8_t	count;
	uint8_t current_display;
	char city[50];
	char longitude[8];
	char latitude[8];
	char current_date[25];
	char sunrise[6];
	char sunset[6];
	forecast_val_t val[MAX_FORECASTDAYS];
};


#define MAX_SENSORS 20

typedef struct sensor_val_t sensor_val_t;
struct sensor_val_t
{
	char    node_name[20];
	char    node_alias[20];
	char    node_type[10];
	char 	temperature[10];
	char 	humidity[10];
	char 	pressure_rel[10];
    char    unixtime[15];
    char    readable_ts[25];
};

typedef struct sensors_t sensors_t;
struct sensors_t
{
	uint8_t	count;
	uint8_t current_display;
	sensor_val_t val[MAX_SENSORS];
};

typedef struct myweather_t myweather_t;
struct myweather_t
{
	uint8_t	present;
	char    timestamp[21];
	char    temperature[7];
	char    humidity[6];
	char 	pressure_rel[7];
	char 	pressure_rising[2];
};


// **********************************************************************
int my_mosquitto_init(char mqtt_host[0], int mqtt_port);

#endif
