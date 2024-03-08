/**
 * @file AVR_SPI_pin_defines.h
 * @author Lukas Ternjej
 *
 * Header file for defining pin registers for a specific device.
 *
 * @date 2024-03-08
 */

#ifndef AVR_SPI_PIN_DEFINES_H_
#define AVR_SPI_PIN_DEFINES_H_

#if defined(__AVR_ATmega88P__) || defined(__AVR_ATmega88PA__) || defined(__AVR_ATmega88__)

    // default SPI pin register defines
    #define SPI_PINx        PINB
    #define SPI_DDRx        DDRB
    #define SPI_PORTx       PORTB

    #define MOSI_PIN_PORTxn PB3     // default MOSI pin defines
    #define MISO_PIN_PORTxn PB4     // default MISO pin defines
    #define SCK_PIN_PORTxn  PB5     // default SCK pin defines
    #define SS_PIN_PORTxn   PB2     // default SS pin defines

#elif defined __AVR_ATmega32__

    // default SPI pin register defines
    #define SPI_PINx        PINB
    #define SPI_DDRx        DDRB
    #define SPI_PORTx       PORTB

    #define MOSI_PIN_PORTxn PB5     // default MOSI pin defines
    #define MISO_PIN_PORTxn PB6     // default MISO pin defines
    #define SCK_PIN_PORTxn  PB7     // default SCK pin defines
    #define SS_PIN_PORTxn   PB4     // default SS pin defines

#endif
#endif