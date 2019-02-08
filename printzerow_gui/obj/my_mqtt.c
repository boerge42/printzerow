/* ************************************************************************
      
         my_mqtt.c
      ================
      Uwe Berger; 2019


---------
Have fun!

************************************************************************* */

#include "my_mqtt.h"

sysinfo_t sysinfo = {0};
sensors_t sensors = {0};	// sensors.count = 0
myweather_t myweather = {0};
forecast_t forecast = {0};

extern menu_pos_t current_menu;
extern uint8_t current_display;

struct mosquitto *mosq	= NULL;


// ************************************************
void get_sensors(sensors_t dest)
{
    memcpy(&dest, &sensors, sizeof(sensors));
}

// ************************************************
void trim(char *str, char ch)
{
    int i;
    int begin = 0;
    int end = strlen(str) - 1;

//    while (isspace((unsigned char) str[begin]))
    while (str[begin] == ch)
        begin++;

//    while ((end >= begin) && isspace((unsigned char) str[end]))
    while ((end >= begin) && str[end] == ch)
        end--;

    // Shift all characters back to the start of the string array.
    for (i = begin; i <= end; i++)
        str[i - begin] = str[i];

    str[i - begin] = '\0'; // Null terminate string.
}

// ************************************************
int replacechar(char *str, char orig, char rep) {
    char *ix = str;
    int n = 0;
    while((ix = strchr(ix, orig)) != NULL) {
        *ix++ = rep;
        n++;
    }
    return n;
}

// ************************************************
void strremove(char* source,char ch) { 
        char* target=source;
        for (;(*target=*source)!=0;source++)
                if (*target!=ch) target++;
}



// ************************************************
void get_json_string(char *dest, struct json_object *j, char *tag)
{
    strcpy(dest, json_object_to_json_string(json_object_object_get(j, tag)));
    trim(dest, '"');
}

// ************************************************
void get_json_array_string(char *dest, struct json_object *j, uint idx)
{
    struct json_object *j_obj;

    j_obj = json_object_array_get_idx(j, idx);
    strcpy(dest, json_object_to_json_string(j_obj));
    trim(dest, '"');
}

// ************************************************
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


// ************************************************
void my_message_callback(struct mosquitto *mosq, void *userdata, const struct mosquitto_message *message)
{
    char **topics;
    int topic_count;
    uint8_t i, idx;
    uint8_t found;

    struct json_object *j_root;
    struct json_object *j_obj;
    struct json_object *j_array;
    struct json_object *j_array_element;

    if(message->payloadlen){
		//printf("%s --> %s\n", (char *)message_l->topic, (char *)message_l->payload);
        mosquitto_error_handling(mosquitto_sub_topic_tokenise(message->topic, &topics, &topic_count));

        // zerow/sysinfo/json 
        // { 
	    //  "hostname": "zerow", 
	    //  "uptime": "2 days, 2:34:21", 
	    //  "load": [ "0.32", "0.25", "0.27" ], 
	    //  "ram": { "free": "234832", "share": "21912", "buffer": "39472", "total": "492620" }, 
	    //  "swap": { "free": "102396", "total": "102396" }, 
	    //  "processes": "108", 
	    //  "time": { "unix": "1536080941", "readable": "2018\/09\/04 19:09:01" } 
        // }
        if (strcmp(topics[1], "sysinfo") == 0) {
            //puts("-->sysinfo!!!");
            found=0;
            idx=0;
            for (i=0; i<sysinfo.count; i++) {
                if (!found && strcmp(sysinfo.val[i].hostname, topics[0])==0) {
                    found=1;
                    idx=i;
                }
            }
            if (!found) {
                idx=sysinfo.count;
                if (sysinfo.count < MAX_SYSINFO) 
                    sysinfo.count++;
            }
            j_root=json_tokener_parse(message->payload);
            get_json_string(sysinfo.val[idx].hostname, j_root, "hostname");
            get_json_string(sysinfo.val[idx].uptime, j_root, "uptime");
            get_json_string(sysinfo.val[idx].processes, j_root, "processes");
            j_array=json_object_object_get(j_root, "load");
            get_json_array_string(sysinfo.val[idx].load_1m, j_array, 0);
            get_json_array_string(sysinfo.val[idx].load_5m, j_array, 1);
            get_json_array_string(sysinfo.val[idx].load_15m, j_array, 2);
            j_obj=json_object_object_get(j_root, "ram");
            get_json_string(sysinfo.val[idx].ram_free, j_obj, "free");
            get_json_string(sysinfo.val[idx].ram_share, j_obj, "share");
            get_json_string(sysinfo.val[idx].ram_buffer, j_obj, "buffer");
            get_json_string(sysinfo.val[idx].ram_total, j_obj, "total");
            j_obj=json_object_object_get(j_root, "swap");
            get_json_string(sysinfo.val[idx].swap_free, j_obj, "free");
            get_json_string(sysinfo.val[idx].swap_total, j_obj, "total");
            j_obj=json_object_object_get(j_root, "time");
            get_json_string(sysinfo.val[idx].timestamp, j_obj, "readable");
            strremove(sysinfo.val[idx].timestamp, '\\');
            if (current_display == DISPLAY_SYSINFO) current_menu.function_display = 1;
            //puts(sysinfo.val[idx].processes);
        } else

        // sensors/xxxx/json
        // {"heap":"23216","temperature":"30.8",
        //  "humidity":"41.7","unixtime":"1533665227",
        //  "node_name":"esp8266-9982412","node_alias":"Bad",
        //  "node_type":"dht22","readable_ts":"2018/08/07 20:07:07"}
        if (strcmp(topics[0], "sensors") == 0) {
            //puts("-->sensors!!!");
		    //printf("%s --> %s\n", (char *)message->topic, (char *)message->payload);
            found=0;
            idx=0;
            for (i=0; i<sensors.count; i++) {
                if (!found && strcmp(sensors.val[i].node_name, topics[1])==0) {
                    found=1;
                    idx=i;
                }
            }
            if (!found) {
                idx=sensors.count;
                if (sensors.count < MAX_SENSORS) 
                    sensors.count++;
            }
            //printf("==>> %i\n", idx);
            j_root=json_tokener_parse(message->payload);
            get_json_string(sensors.val[idx].node_name, j_root,    "node_name");
            get_json_string(sensors.val[idx].node_alias, j_root,   "node_alias");
            get_json_string(sensors.val[idx].node_type, j_root,    "node_type");
            get_json_string(sensors.val[idx].temperature, j_root,  "temperature");
            get_json_string(sensors.val[idx].pressure_rel, j_root, "pressure_rel");
            get_json_string(sensors.val[idx].humidity, j_root,     "humidity");
            get_json_string(sensors.val[idx].unixtime, j_root,     "unixtime");
            get_json_string(sensors.val[idx].readable_ts, j_root,  "readable_ts");
            if (current_display == DISPLAY_ALL_SENSORS) current_menu.function_display = 1;
        } else

        // myweather/all/json
        // {"timestamp":"02.09.2018, 12:59:49",
        //  "temperature_out":"18.7",
        //  "temperature_in":"20.2",
        //  "humidity_in":"77.3",
        //  "humidity_out":"73.0",
        //  "pressure_rel":"1025.2",
        //  "pressure_rising":"0"}
        // sysinfo?
        if (strcmp(topics[0], "myweather") == 0) {
            j_root=json_tokener_parse(message->payload);
            get_json_string(myweather.timestamp, j_root,   "timestamp");
            get_json_string(myweather.temperature, j_root, "temperature_out");
            get_json_string(myweather.humidity, j_root, "humidity_out");
            get_json_string(myweather.pressure_rel, j_root, "pressure_rel");
            get_json_string(myweather.pressure_rising, j_root, "pressure_rising");
            myweather.present = 1;
            if (current_display == DISPLAY_MYWEATHER) current_menu.function_display = 1;
        } else

        // weatherforecast/all/json 
        // {
        //     "city":"Brandenburg",
        //     "longitude":"12.56",
        //     "latitude":"52.41",
        //     "current_date":"Sun, 02.09.2018, 12:07",
        //     "sunrise":"06:23",
        //     "sunset":"19:54",
        //     "forecast":[
		//         {"day":"Sun","date":"02.09.2018","temp_low":"12","temp_high":"20","text":"Showers","code":"11"},
		//         {"day":"Mon","date":"03.09.2018","temp_low":"15","temp_high":"27","text":"Mostly Cloudy","code":"28"},
		//         {"day":"Tue","date":"04.09.2018","temp_low":"16","temp_high":"25","text":"Mostly Sunny","code":"34"},
		//         {"day":"Wed","date":"05.09.2018","temp_low":"13","temp_high":"23","text":"Partly Cloudy","code":"30"},
		//         {"day":"Thu","date":"06.09.2018","temp_low":"13","temp_high":"24","text":"Rain","code":"12"},
		//         {"day":"Fri","date":"07.09.2018","temp_low":"13","temp_high":"20","text":"Mostly Cloudy","code":"28"},
		//         {"day":"Sat","date":"08.09.2018","temp_low":"11","temp_high":"23","text":"Partly Cloudy","code":"30"}
	    //     ]
        // }
        if (strcmp(topics[0], "weatherforecast") == 0) {
            j_root=json_tokener_parse(message->payload);
            get_json_string(forecast.city, j_root, "city");
            get_json_string(forecast.longitude, j_root, "longitude");
            get_json_string(forecast.latitude, j_root, "latitude");
            get_json_string(forecast.current_date, j_root, "current_date");
            get_json_string(forecast.sunrise, j_root, "sunrise");
            get_json_string(forecast.sunset, j_root, "sunset");
            j_array=json_object_object_get(j_root, "forecast");
            for (i=0; i < MAX_FORECASTDAYS; i++) {
                j_array_element = json_object_array_get_idx(j_array, i);
                get_json_string(forecast.val[i].day, j_array_element, "day");
                get_json_string(forecast.val[i].date, j_array_element, "date");
                get_json_string(forecast.val[i].temp_low, j_array_element, "temp_low");
                get_json_string(forecast.val[i].temp_high, j_array_element, "temp_high");
                get_json_string(forecast.val[i].text, j_array_element, "text");
                get_json_string(forecast.val[i].code, j_array_element, "code");
            }
            forecast.count = MAX_FORECASTDAYS;
            if (current_display == DISPLAY_FORECAST) current_menu.function_display = 1;
        }
        
        mosquitto_error_handling(mosquitto_sub_topic_tokens_free(&topics, topic_count));

	}else{
		printf("%s (null)\n", message->topic);
	}

}


// ************************************************
int my_mosquitto_init(char mqtt_host[50], int mqtt_port)
{
    
    mosq = mosquitto_new(NULL, true, NULL);
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
        return 1;
    }

	// Callbacks definieren
    //mosquitto_log_callback_set(mosq, my_log_callback);
	mosquitto_message_callback_set(mosq, my_message_callback);   

	// mit MQTT-Broker verbinden
    mosquitto_error_handling(mosquitto_connect(mosq, mqtt_host, mqtt_port, MQTT_KEEPALIVE));

	// Topic abonnieren
	//mosquitto_error_handling(mosquitto_subscribe(mosq, NULL, "+/+/json", 0));
	mosquitto_error_handling(mosquitto_subscribe(mosq, NULL, "sensors/+/json", 0));
	mosquitto_error_handling(mosquitto_subscribe(mosq, NULL, "myweather/all/json", 0));
	mosquitto_error_handling(mosquitto_subscribe(mosq, NULL, "weatherforecast/all/json", 0));
	mosquitto_error_handling(mosquitto_subscribe(mosq, NULL, "+/sysinfo/json", 0));
    
    // ...und Feuer
    mosquitto_loop_start(mosq);




    return 0;
}
