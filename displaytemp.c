/*
 * Module...: MCP9808example1.c
 * Version..: 0.1 (Beta - unfinished)
 * Author...: Henrik Thomsen/Mercantec <heth@mercantec.dk
 * Date.....: 24. nov 2020
 *
 * Abstract.: Implementation of Seeed Grove Sensor "I2C High
 *            Accuracy Temperature sensor" based on MCP-9808
 *            Implementet in user space using libi2c
 * Mod. log.: 01-06-2021: Could not compile. Got i2c_smbus_read_word_data undefined.
                              Include <i2c/smbus.h> should be include. (Worked in 2020)

 * License..: Free open software but WITHOUT ANY WARRANTY.
 * Terms....: see http://www.gnu.org/licenses
 *
 * Documentation - see https://mars.merhot.dk/w/index.php/Grove_I2C_High_Accuracy_Temerature_Sensor_-_Seeed
 *
 * REMEMBER: Install the libi2c-dev package to compile this file
 * Compile this example with: gcc -li2c i2c_MCP9808-example1.c -o i2c_MCP9808-example1
*
 * See also: https://www.kernel.org/doc/Documentation/i2c/dev-interface
 */
#include <stdio.h>
#include <unistd.h>
#include <stdint.h>
#include <fcntl.h>
#include <sys/ioctl.h>
#include <stdlib.h>
#include <errno.h>
#include <string.h>
#include <linux/i2c-dev.h>
#include <i2c/smbus.h>        // Added 01062021 HeTh
#include <byteswap.h>
#include <syslog.h>
#include "controlDisplay.h"
#include "daemonopgave.h"

/* The MCP9808 Digital temperature sensor IC from Microchip is a i2c/SMBus compatible
 * device. To understand this code you need access to MCP9808 Datasheet from Microchip
 *
 * The device is implementet on Seeed Studios Grove I2C high accuracy Temperature Sensor
 *
 * Links and supplemental information can be found on tthe documentation link in this
 * files header.
 *
 * The MCP9808 contains severel registers:
 *
 * Register addresses:
 *  0x00: RFU    - (Reserved for future use) Read only
 *  0x01: CONFIG - Configuration register
 *  0x02: Tupper - Alert temperature Upper boundary trip register
 *  0x03: Tlower - Alert temperature Lower boundary trip register
 *  0x04: Tcrit  - Critical temperature trip register
 *  0x05: Ta     - Temperature register
 *  0x06:        - Manufacurer ID register (Reads 0x54)
 *  0x07:        - Device ID/Revision register
 *  0x08:        - Resolution register
 *  0x09-0x0F    - Reserved....
 */

#define CONFIG_REG       0x01
#define TUPPER_REG       0x02
#define TLOWER_REG       0x03
#define TCRIT_REG        0x04
#define TA_REG           0x05
#define MANID_REG        0x06
#define DEVID_REG        0x07
#define RES_REG          0x08
#define TEMP_BITMASK_HEX 0x0FFF

//Device specific information. (Perhaps better as command options or config file)
#define MPC9808_BUS      "/dev/i2c-2"
#define MPC9808_ADR      0x18

/*************************** FUNCTION HEADER ******************************
Name....: i2c_init
Author..: Mads Meyer
***************************************************************************
Abstract: Funktion til at initialisere i2c slaven
Inputs..: char *bus, unsigned int address
Outputs.: int
***************************************************************************
Modification log:
**************************************************************************/
int i2c_init(char *bus, unsigned int address) {
        int file;

        file = open(bus, O_RDWR);
        if (file < 0) { // If error
                fprintf(stderr, "ERROR: opening %s - %s\n", bus, strerror(errno));
                exit(1);
        }

        if (ioctl(file, I2C_SLAVE, address) == -1 ) { // If error
             fprintf(stderr, "ERROR: setting  address %d on i2c bus %s with ioctl() - %s", address, bus, strerror(errno) );
             exit(1);
        }
        return(file);
}

/*************************** FUNCTION HEADER ******************************
Name....: printBits
Author..: Mads Meyer
***************************************************************************
Abstract: Funktion til at udkskrive bits samt bitmaske, for at visualisere inputtet
Inputs..: int bits
Outputs.: int
***************************************************************************
Modification log:
**************************************************************************/
int printBits(int bits) {
        // Looper igennem 16 bits af input (bits)
        for (int i = 15; i >= 0; i--)
        {

                if(bits & (1 << i)) {
                        printf("1");
                } else {
                        printf("0");
                }

                if(i % 4 == 0) {
                        printf(" ");
                }
        }
        printf("\n");
        return 0;
}


/*************************** FUNCTION HEADER ******************************
Name....: temp_get
Author..: Mads Meyer
***************************************************************************
Abstract: Funktion til at hente temperaturen
Inputs..: none
Outputs.: int
***************************************************************************
Modification log:
**************************************************************************/
char* temp_get() {
        int file;
        file = i2c_init(MPC9808_BUS, MPC9808_ADR);
        int32_t reg32;
        uint16_t * const reg16poi = (uint16_t *) &reg32; //Address reg32 wordwise
        uint8_t  * const reg8poi  = (uint8_t *)  &reg32; //Address reg32 bytewise

        // Read manufactorer ID
        // Note: i2c_smbus_read_word_data returns in big-endian
        //       bit 0-7 = reg8poi[1] and bit 8-15 = reg8poi[0]
        reg32 = i2c_smbus_read_word_data(file, MANID_REG);
        if ( reg32 < 0 ) {
                fprintf(stderr, "ERROR: Read failed  on i2c bus register %d - %s\n",  MANID_REG,strerror(errno) );
                exit(1);
        }
        if ( bswap_16(reg16poi[0]) != 0x0054 ) { // Check manufactorer ID - Big endian 5400 and not 0054
                fprintf(stderr, "Manufactorer ID wrong is 0x%x should be 0x54\n",__bswap_16(reg16poi[0]));
                exit(1);
        }
        // Read device ID and revision
        reg32 = i2c_smbus_read_word_data(file, DEVID_REG);
        if ( reg32 < 0 ) {
                fprintf(stderr, "ERROR: Read failed  on i2c bus register %d - %s\n",  DEVID_REG,strerror(errno) );
                exit(1);
        }
        if ( reg8poi[0] != 0x04 ) { // Check device ID - Big endian 0400 and not 0004
                fprintf(stderr, "Manufactorer ID OK but device ID wrong is 0x%x should be 0x4\n",reg8poi[0]);
                exit(1);
        }
        //revision = reg8poi[1];
/*DEBUG*/       printf("All good :-)\n");
        
        // Read temperature...

        // Læser data fra TA_REG registeret ved hjælp af I2C kommunikation.
        reg32 = i2c_smbus_read_word_data(file, TA_REG);
        // Tjekker om der er sket en fejl under læsning. hvis reg32 er mindre end 0 så er der sket en fejl
        if (reg32 < 0) {
        // Sender fejl besked til standard error
        fprintf(stderr, "Error reading register \n");
        exit(1);
        }

        // Omdanner den rå temperaturværdi fra big-endian format til little-endian format
        int rawTemp = bswap_16(reg16poi[0]);

        // Dividere med 16 pga temperaturen er i 16-bit
        double tempCelsius = (rawTemp & TEMP_BITMASK_HEX) / 16.0; // Convert raw value to Celsius
        printf("BitMask:          ");
        printBits(TEMP_BITMASK_HEX);

        printf("Raw Value:        ");
        printBits(rawTemp);

        printf("Raw temp Bits:    ");
        printBits((rawTemp & TEMP_BITMASK_HEX));

        char *screenString = malloc(20 * sizeof(char)); // Allocate memory dynamically
        sprintf(screenString, "Temp: %.2f", tempCelsius);
        printf("Temperature: %.2f°\n", tempCelsius);
        return(screenString);

}

