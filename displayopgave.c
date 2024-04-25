#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>
#include <unistd.h>
#include <syslog.h>
#include "displaytemp.h"
#include "displaydisplay.h"
#include "displayutils.h"
#include "displaymqtt.h"

/*Abbreviation Table
time_string          == Denne streng gemmer tiden globalt, da den skal bruges i forskellige func
temperature_string   == Denne streng gemmer temperatur globalt, da den skal bruges i forskellige func
blink_state          == Denne int hjælper med at skifte ":" til on/off
*/

pthread_mutex_t mutex;
char *time_string = NULL;
char *temperature_string = NULL;
int blink_state = 0;
int mqtt_publish_checker = 0;

/*************************** FUNCTION HEADER ******************************
Name....: update_time
Author..: Mads Meyer
***************************************************************************
Abstract...: Function is used for updating time section on the groove display
Parameters.: none 
Outputs....: void
***************************************************************************
Modification log:
**************************************************************************/
void *update_time() {
    while (1) {
        pthread_mutex_lock(&mutex);
        char *current_time = get_current_time_string();
        if (time_string != NULL) {
            free(time_string);
        }
        time_string = current_time;
            if(blink_state == 1) {
                time_string[2] = ' ';
                blink_state = 0;
            }
            else {
                time_string[2] = ':';
                blink_state = 1;
            }
        char output_string[20];
        sprintf(output_string, "%s %s", time_string, temperature_string);
        display_print_string(output_string, 2);
        pthread_mutex_unlock(&mutex);
        printf("ur blinker!\n");
        usleep(500000); // Sleep for 0.5 seconds
    }
    return NULL;
}

/*************************** FUNCTION HEADER ******************************
Name....: update_temperature
Author..: Mads Meyer
***************************************************************************
Abstract...: Function is used for updating temparture section on the groove display
Parameters.: none 
Outputs....: void
***************************************************************************
Modification log:
**************************************************************************/
void *update_temperature() {
    while (1) {
        pthread_mutex_lock(&mutex);
        char *temp = temp_get();
        if (temperature_string != NULL) {
            free(temperature_string);
        }
        temperature_string = temp;
        char output_string[20];
        sprintf(output_string, "%s | %s", time_string, temperature_string);
        display_print_string(output_string, 2);
        pthread_mutex_unlock(&mutex);
        printf("Temp opdateret!!\n");
        syslog(LOG_INFO, output_string);
        if(mqtt_publish_checker == 2) {
            publish_message(output_string);
            mqtt_publish_checker = 0;
        }
        else {
            mqtt_publish_checker++;
        }
        usleep(10000000); // Sleep for 10 seconds
    }
    return NULL;
}

/*************************** FUNCTION HEADER ******************************
Name....: update_temperature
Author..: Mads Meyer
***************************************************************************
Abstract...: Main function is responsible for fetching data to the display,
             assigning different functions to threads and freeing up unused resources
Parameters.: none 
Outputs....: int
***************************************************************************
Modification log:
**************************************************************************/
int main(void) {
    display_init();
    init_mqtt();
    // Henter ip, hvis den fejler, prøver den i 20sek at hente den
    int timer = 0;
    char* ip = get_ip_address();
    
    // Fejl håndtering
    if(ip == NULL) {
        while(ip == NULL && timer > 10) {
            ip = get_ip_address();
            usleep(1000000); // Sleep for 1 seconds
            timer++;
        }
    }
    display_print_string(ip, 1);

    // Åbner syslog
    openlog("TempMeasurement", LOG_PID, LOG_USER);
    pthread_t time_thread, temp_thread;
    pthread_mutex_init(&mutex, NULL);

    // Laver tråde for at opdatere tid og temperatur
    pthread_create(&time_thread, NULL, update_time, NULL);
    pthread_create(&temp_thread, NULL, update_temperature, NULL);

    // Joiner trådene / Venter på, at trådene færdiggør deres arbejde
    pthread_join(time_thread, NULL);
    pthread_join(temp_thread, NULL);

    // Rydder op: destruerer mutexen
    pthread_mutex_destroy(&mutex);

    // Frigør hukommelse allokeret til tids og temperatur strengene, hvis de ikke er NULL
    if (time_string != NULL) {
        free(time_string);
    }
    if (temperature_string != NULL) {
        free(temperature_string);
    }

    return 0;
}
