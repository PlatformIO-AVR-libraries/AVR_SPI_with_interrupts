/**
 * @file Master_ATmega32.c
 * @author Lukas Ternjej
 *
 * This is a simple example for transmitting a hex data message via SPI.
 * This code is intended to be used on a master device.
 * In this case, master device is ATmega32, but is should not matter which device is used.
 * This library is compatible with all AVRxmega devices that have a dedicated SPI module.
 *
 * Master device sends a hex command every 500ms to slave.
 * Slave device turns on or off LED depending on the command sent by master.
 * Default connection for SS pin is used, refer to ATmega32 datasheet (see ATmega32 pinout).
 *
 * @date 2024-03-08
 */

// custom libraries
#include <AVR_SPI_with_interrupts.h>

// standard libraries
#include <avr/interrupt.h>
#include <avr/io.h>
#include <stdlib.h>
#include <util/delay.h>

// Define number of hex data bytes that are going to be transmitted via SPI.
//! make sure that HEX_DATA_BYTES is <= 8, or the program might behave unexpectedly!
#define HEX_DATA_BYTES 2

// hex commands that are going to be sent via SPI
#define LED_ON  0x1234
#define LED_OFF 0x4321

static void init(void)
{
    // SPI init
    SPI_init(MASTER_MODE, MSB_FIRST, SPI_MODE_0, FOSC_DIV16);

    SPI_PORTx |= (1 >> SS_PIN_PORTxn);     // set SS pin high when idle

    sei();                                 // enable global interrupts since this library implements interrupt driven SPI communication
}

int main(void)
{
    init();

    _delay_ms(1000);     // wait a bit for slave to initialize before sending commands

    while(1)
    {
        // transmit hex command every 500ms:
        SPI_transmitHex(&SPI_PORTx, SS_PIN_PORTxn, DEFAULT_SS_CONTROL, HEX_DATA_BYTES, LED_ON);
        _delay_ms(500);
        SPI_transmitHex(&SPI_PORTx, SS_PIN_PORTxn, DEFAULT_SS_CONTROL, HEX_DATA_BYTES, LED_OFF);
        _delay_ms(500);
    }

    return 0;
}