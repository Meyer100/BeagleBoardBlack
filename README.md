# BeagleBoardBlack

## Build the project
```c
gcc displayopgave.c displaytemp.c displaydisplay.c displayutils.c displaymqtt.c -o displayopgave -li2c -pthread -lmosquitto
```

### DisplayTemp api
This file returns the temperature from the MCP9808
```c
extern char* temp_get();
```

### DisplayDisplay api
This file is responsible for printing out string to screen
```c
extern int display_init();
extern void display_print_string(const char *string, int line);
```

### DisplayUtils api
This file returns the api address (eth0) and current local time (+2 hours)
```c
extern char* get_ip_address();
extern char* get_current_time_string();
```

### DisplayMqtt api
This file is responsible for sending out a message to the broker
```c
extern int init_mqtt();
extern void publish_message(const char *payload);
```

## Implementing Daemon
Change startup file
```
sudo nano /etc/systemd/system/displayopgave.service
```
file should look like this
```
[Unit]
Description=My led display
After=network.target

[Service]
ExecStart=/home/debian/bin/filename
Type=simple
Restart=always


[Install]
WantedBy=default.target
```
Reload the systemd manager configuration
```
sudo systemctl daemon-reload
```
Enable service
```
sudo systemctl enable myprogram.service
```
Start service
```
sudo systemctl start myprogram.service
```
Check status
```
systemctl status myprogram.service
```

## Display.c

### Updating time intervals
Change `usleep(value)` to desired time (mesaured in microseconds)
#### example
```c
usleep(30000000) // Sleep for 30 seconds
```
Change intervals between sending message to broker
```c
// This loop waits 30 seconds for the message to be sent
if(mqtt_publish_checker == 2) {
  publish_message(output_string);
  mqtt_publish_checker = 0;
}
  else {
  mqtt_publish_checker++;
}

````
Example of how to change it
```c
// This loop waits 50 seconds for the message to be sent
if(mqtt_publish_checker == 4) {
  publish_message(output_string);
  mqtt_publish_checker = 0;
}
  else {
  mqtt_publish_checker++;
}

````
