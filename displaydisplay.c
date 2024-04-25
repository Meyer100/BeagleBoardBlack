#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <fcntl.h>
#include <sys/ioctl.h>
#include <linux/i2c-dev.h>
#include <string.h>

#define I2C_BUS "/dev/i2c-2"
#define I2C_ADDR 0x3e

#define LED_CHIP_ADDR 0x62

int i2c_fd;

/*************************** FUNCTION HEADER ******************************
Name....: i2c_write_byte
Author..: Mads Meyer
***************************************************************************
Abstract: Funktion til at skrive en byte til I2C-bussen
Inputs..: unsigned char reg, unsigned char value
Outputs.: void
***************************************************************************
Modification log:
**************************************************************************/
void i2c_write_byte(unsigned char reg, unsigned char value) {
    unsigned char buffer[2];
    buffer[0] = reg;
    buffer[1] = value;
    if (write(i2c_fd, buffer, 2) != 2) {
        printf("Failed to write to the i2c bus.\n");
        exit(1);
    }
}

/*************************** FUNCTION HEADER ******************************
Name....: display_print_string
Author..: Mads Meyer
***************************************************************************
Abstract: Funktion til at udskrive en streng på displayet
Inputs..: none
Outputs.: void
***************************************************************************
Modification log:
**************************************************************************/
void display_print_string(const char *string, int line) {    
    if (line == 2) {
        i2c_write_byte(0x00, 0xC0); 
    }

    for (int i = 0; i < strlen(string); i++) {
        // Write the character to the display
        i2c_write_byte(0x40, string[i]);
    }
}

/*************************** FUNCTION HEADER ******************************
Name....: display_init
Author..: Mads Meyer
***************************************************************************
Abstract: Funktion til at initialisere displayet
Inputs..: none
Outputs.: int
***************************************************************************
Modification log:
**************************************************************************/
int display_init() {
    char *filename = I2C_BUS;
    if ((i2c_fd = open(filename, O_RDWR)) < 0) {
        perror("Failed to open the i2c bus.\n");
        exit(1);
    }
    if (ioctl(i2c_fd, I2C_SLAVE, I2C_ADDR) < 0) {
        perror("Failed to acquire bus access and/or talk to slave.\n");
        exit(1);
    }

    // Function set: 2 line mode and 5x8
    i2c_write_byte(0x00, 0x28);
    // Display on/off control: Display on, cursor on, blink off
    i2c_write_byte(0x00, 0x0D);
    // Display clear
    i2c_write_byte(0x00, 0x01);
    // Entry mode set
    i2c_write_byte(0x00, 0x06);
    // Return home
    i2c_write_byte(0x00, 0x02);

    // Turn of the sleep mode
    //i2c_write_byte(0x00, 0x00);

    // LEDOUT = 0x08
    //i2c_write_byte(0x08, 0x15);

    return 0;

}
