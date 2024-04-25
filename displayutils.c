#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>

#define BUFFER_SIZE 128

/*************************** FUNCTION HEADER ******************************
Name....: get_ip_address
Author..: Mads Meyer
***************************************************************************
Abstract: Funktion hente ip addressen
Inputs..: none
Outputs.: char*
***************************************************************************
Modification log:
**************************************************************************/
char* get_ip_address() {
    char command[256];
    char buffer[BUFFER_SIZE];
    char* result = malloc(BUFFER_SIZE);
    if (result == NULL) {
        fprintf(stderr, "Memory allocation failed\n");
        return NULL;
    }

    sprintf(command, "ip addr show %s | grep 'inet ' | awk '{print $2}' | cut -d/ -f1", "eth0");

    // Åbner kommandoen til læsning
    FILE *fp = popen(command, "r");
    if (fp == NULL) {
        fprintf(stderr, "Failed to run command\n");
        free(result);
        return NULL;
    }

    // Læser outputtet en linje ad gangen
    if (fgets(buffer, sizeof(buffer)-1, fp) != NULL) {
        // Kopierer strengen til resultatet
        strcpy(result, buffer);
        // Fjerner eventuelle newline tegn hvis de er i strengen
        result[strcspn(result, "\n")] = 0;
    } else {
        fprintf(stderr, "Failed to retrieve IP address\n");
        free(result);
        result = NULL;
    }

    // close
    pclose(fp);
    return result;
}

/*************************** FUNCTION HEADER ******************************
Name....: get_current_time_string
Author..: Mads Meyer
***************************************************************************
Abstract: Funktion hente akutelle lokale tid
Inputs..: none
Outputs.: char*
***************************************************************************
Modification log:
**************************************************************************/
char* get_current_time_string() {
    time_t now;
    struct tm *local_time;

    // Henter den aktuelle tid
    time(&now);
    now += 2 * 3600;  // Tilføjer 2 timer

    // Henter den lokale tid
    local_time = localtime(&now);

    // Allokerer hukommelse til tidsstrengen
    char* time_string = malloc(64 * sizeof(char));
    if (time_string == NULL) {
        fprintf(stderr, "Memory allocation failed\n");
        return NULL;
    }

    // Formatterer strengen
    if (strftime(time_string, 64, "%H:%M", local_time) == 0) {
        fprintf(stderr, "Failed to format time\n");
        free(time_string);
        return NULL;
    }

    return time_string;
}
