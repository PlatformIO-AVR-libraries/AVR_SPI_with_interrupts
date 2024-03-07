/**
 * @file Slave_ATmega88.cpp
 * @author Lukas Ternjej
 *
 * This is a simple example for receiving a hex data message via SPI.
 * This code is intended to be used on a slave device.
 * In this case, slave device is ATmega88, but is should not matter which device is used.
 * This library is compatible with all AVRxmega devices that have a dedicated SPI module.
 *
 * Default connection for SS pin is used, refer to ATmega88 datasheet (see ATmega88 pinout).
 * Slave device receives a hex command from master.
 * Depending on received command, led is turned on or off.
 *
 * LED with a series 220R resistor is connected from PORTC5 to GND.
 *
 * @date 2024-03-07
 */

// Define number of hex data bytes that are going to be received via SPI.
// It is not necessary to define [HEX_DATA_BYTES] for slave device, but it will
// prevent a compiler warning.
#define HEX_DATA_BYTES 2

// custom libraries
#include <AVR_SPI_with_interrupts.h>

// standard libraries
#include <avr/interrupt.h>
#include <avr/io.h>
#include <stdlib.h>
#include <string.h>
#include <util/delay.h>

// define led ports
#define LED_DDRx   DDRC
#define LED_PORTx  PORTC
#define LED_PORTxn PC5

// string command that is going to be sent via SPI
#define STRING_COMMAND "TOGGLE"

static void init()
{
    // SPI init
    SPI_init(SLAVE_MODE, MSB_FIRST, SPI_MODE_0, FOSC_DIV16);

    LED_DDRx |= (1 << LED_PORTxn);     // set led pin as output

    sei();                             // enable global interrupts since this library implements interrupt driven SPI communication
}

int main(void)
{
    init();

    while(1)
    {
        if(SPI_readAll() == true)     // check if [DATA_END_CHAR] is reached
        {
            // compare data from SPI_data[] buffer with [STRING_COMMAND]
            if(!strcmp((char *)SPI_data, STRING_COMMAND))
                LED_PORTx ^= (1 << LED_PORTxn);     // toggle LED when receiving command
        }
    }

    return 0;
}