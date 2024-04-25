#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <mosquitto.h>

//Author: heth@mercantec.dk
//Date..: 19. nov 2020
//Rev...: 11. april 2024
//Ver...: 0.1 beta
//
//
//INFORMATION
//Mosquitto docementation:
//   - https://mosquitto.org/documentation/
//   - https://github.com/eclipse/mosquitto (Se under exanples)
//Compile with : gcc mqtt_ex1.c -o mqtt_ex1 -lmosquitto

int number;
struct mosquitto *mosq;

/*************************** FUNCTION HEADER ******************************
Name....: publish_message
Author..: Mads Meyer
***************************************************************************
Abstract...: Function can publish a message to the broker
Parameters.: const char *payload 
Outputs....: void
***************************************************************************
Modification log:
**************************************************************************/
void publish_message(const char *payload) {
    int rc = mosquitto_publish(mosq, NULL, "Mads/celcius", strlen(payload), payload, 2, true);
    if (rc != MOSQ_ERR_SUCCESS) {
        fprintf(stderr, "Failed to publish message: %s\n", mosquitto_strerror(rc));
    }
    else {
        printf("Message sent to broker!\n");
        printf("--- Data Sent to broker ---!\n");
        printf("%s\n", payload);
        printf("--- Data Sent to broker ---!\n");

    }
}

/*************************** FUNCTION HEADER ******************************
Name....: on_connect1
Author..: Mads Meyer
***************************************************************************
Abstract...: Function connects to the broker. printing to standarderror if it fails
Parameters.: struct mosquitto *mosq, void *obj, int result 
Outputs....: void
***************************************************************************
Modification log:
**************************************************************************/
void on_connect1(struct mosquitto *mosq, void *obj, int result)
{
    int rc = MOSQ_ERR_SUCCESS;

    if(!result){
        mosquitto_subscribe(mosq, NULL, "#", 0);
    }else{
        fprintf(stderr, "%s\n", mosquitto_connack_string(result));
    }
}

/*************************** FUNCTION HEADER ******************************
Name....: on_message1
Author..: Mads Meyer
***************************************************************************
Abstract...: Function listens for messages from the broker
Parameters.: struct mosquitto *mosq, void *obj, const struct mosquitto_message *message 
Outputs....: void
***************************************************************************
Modification log:
**************************************************************************/
void on_message1(struct mosquitto *mosq, void *obj, const struct mosquitto_message *message)
{
    struct mosquitto *mosq2 = (struct mosquitto *)obj;
        char *txtpoi, *txtpoi2;
        printf("struct mosquitto_message contains:\n");
        printf("  mid (int).............: %i\n", message->mid );
        printf("  topic (* char)........: %s\n", message->topic );
        printf("  payload (* void)......: %p", message->payload );
        txtpoi = malloc(message->payloadlen + 1);
        if ( txtpoi == 0 ) {
                fprintf( stderr, "Malloc error\n");
        } else {
                strncpy(txtpoi, message->payload, message->payloadlen);
                txtpoi[message->payloadlen] = 0;
                printf("  Message: [%s]\n", txtpoi);
        }
        printf("  payloadlen (int)......: %i\n", message->payloadlen );
        printf("  qos (int).............: %i\n", message->qos );
        printf("  retain (int)..........: %i\n", message->retain );

        txtpoi2 = malloc( message->payloadlen + 20);
        sprintf(txtpoi2, "#%i:%s", number, txtpoi);
        
        // Publish henrik server
        //mosquitto_publish(mosq2, NULL, "mads/celcius", strlen(str), str, message->qos, message->retain);

        free(txtpoi2);
        free(txtpoi);
}

/*************************** FUNCTION HEADER ******************************
Name....: init_mqtt
Author..: Mads Meyer
***************************************************************************
Abstract...: Function initializes the mqtt connection
Parameters.: none 
Outputs....: void
***************************************************************************
Modification log:
**************************************************************************/
int init_mqtt() {
    int version[3];
    number = 1; // Init message number

    mosquitto_lib_init();
    mosquitto_lib_version(&version[0],&version[1],&version[2]);
    printf("Mosquitto library version. %i.%i.%i\n", version[0], version[1], version[2]);

    mosq = mosquitto_new(NULL, true, NULL);

    mosquitto_connect_callback_set(mosq, on_connect1);
    //mosquitto_message_callback_set(mosq, on_message1);

    mosquitto_connect(mosq, "93.166.84.21", 1883, 60);  // Replace localhost with IP address of broker

    mosquitto_loop_start(mosq); // Start the MQTT client loop asynchronously
    //mosquitto_loop_forever(mosq, -1, 1);

    //mosquitto_destroy(mosq);
    //mosquitto_lib_cleanup();

    return 0;
}
