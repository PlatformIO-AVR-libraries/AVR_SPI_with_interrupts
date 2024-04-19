/**
 * @file Master_ATmega32.c
 * @author Lukas Ternjej
 *
 * This is a simple example for transmitting string data message via SPI.
 * This code is intended to be used on a master device.
 * In this case, master device is ATmega32, but is should not matter which device is used.
 * This library is compatible with all AVRxmega devices that have a dedicated SPI module.
 *
 * Master device sends a string command every 500ms to slave.
 * Slave device toggles an LED when receiving the message.
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

    // string that is going to be transmitted via SPI
    char command[] = "TOGGLE";

    _delay_ms(1000);     // wait a bit for slave to initialize before sending commands

    while(1)
    {
        // transmit string command every 500ms:
        SPI_transmitString(&SPI_PORTx, SS_PIN_PORTxn, DEFAULT_SS_CONTROL, command);
        _delay_ms(500);
    }

    return 0;
}