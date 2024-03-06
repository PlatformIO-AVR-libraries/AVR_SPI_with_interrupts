/**
 * @file AVR_SPI_with_interrupts.h
 * @author Lukas Ternjej
 *
 * Header file that is going to be included in main project file
 * Defines all necessary functions, constants and variables.
 *
 * @date 2024-03-06
 */

#ifndef AVR_SPI_WITH_INTERRUPTS_H_
#define AVR_SPI_WITH_INTERRUPTS_H_

#include "AVR_SPI_char_defines.h"
#include "AVR_SPI_pin_defines.h"

#include <avr/interrupt.h>
#include <avr/io.h>
#include <stdbool.h>
#include <stdlib.h>
#include <string.h>
#include <util/delay.h>

// bit order
#define LSB_FIRST 0x20     // start data transmission with least significant bit first
#define MSB_FIRST 0x0      // start data transmission with most significant bit first

// Master/slave mode
#define MASTER_MODE 1     // SPI device in master mode
#define SLAVE_MODE  0     // SPI device in slave mode

// SPI mode
#define SPI_MODE_0 0x0      // clock logic low when idle, data sampled on rising edge, shifted out on falling edge
#define SPI_MODE_1 0x04     // clock logic low when idle, data sampled on falling edge, shifted out on rising edge
#define SPI_MODE_2 0x08     // clock logic high when idle, data sampled on rising edge, shifted out on falling edge
#define SPI_MODE_3 0x0C     // clock logic high when idle, data sampled on falling edge, shifted out on rising edge

// SPI clock rate
#define FOSC_DIV4   0x0      // FCPU/4
#define FOSC_DIV16  0x01     // FCPU/16
#define FOSC_DIV64  0x02     // FCPU/64
#define FOSC_DIV128 0x03     // FCPU/128

// these SPI clock rates require setting SPI2X bit in SPSR register and specific bits in SPCR
#define FOSC_DIV2  FOSC_DIV4      // FCPU/2
#define FOSC_DIV8  FOSC_DIV16     // FCPU/8
#define FOSC_DIV32 FOSC_DIV64     // FCPU/32

// default or inverted SS line controll
#define INVERTED_SS_CONTROL 0
#define DEFAULT_SS_CONTROL  1

// warn if user didn't manually set [HEX_DATA_BYTES]
#ifndef HEX_DATA_BYTES
    #warning "[HEX_DATA_BYTES] is not defined in main file! It will default to 8."
    #define HEX_DATA_BYTES 8     // maximum of 8 bytes for hex message
#endif

// warn if user set more than 8 data bytes which is outside uint64_t variable size limit
#if HEX_DATA_BYTES > 8
    #error "[HEX_DATA_BYTES] can't be more than 8!"
#endif

/**
 * Function for initializing SPI communication on Atmel AVR ICs that have a dedicated SPI module.
 ** This function doesn't handle multiple slave devices, manual control of multiple SS lines is mandatory.
 *! Make sure that SPI clock frequency that master generates is less than slave device F_CPU/4!!!!
 *
 * @param deviceMode master or slave mode
 * @param dataOrder least or most significant bit first
 * @param SPI_mode SPI mode 0, 1, 2 or 3
 * @param clockRate master SPI clock rate
 */
void SPI_init(uint8_t deviceMode, uint8_t dataOrder, uint8_t SPIMode, uint8_t clockRate);

/**
 * Function that returns a character from master SPDR register
 * Write dummy data to SPDR register to generate SCK for transmission
 *
 * @return returns a char from SPSR register
 */
char SPI_masterReadChar();

/**
 * Function that returns a character from SPDR register
 *
 * @return returns a char from SPSR register
 */
char SPI_readChar();

/**
 * Function that flushes a buffer and sets all array elements to '\0'
 *
 * @param array array to be flushed
 */
void flushBuffer(char array[]);

/**
 * Function that checks if SPI data transmiossion is complete.
 *
 * @return Returns true if [STRING_END_CHAR] is reached, else return false
 */
bool SPI_readAll(void);

/**
 * Function that writes a char in SPDR register. When in master mode,
 * writing to the SPDR register generates SPI clock.
 *
 * @param c character that is going to be written to SPDR register
 */
void SPI_masterPutChar(char c);

/**
 * Writes a character to SPDR register
 *
 * @param c character that is going to be written to SPDR register
 */
void SPI_putChar(char c);

/**
 * Function for transmitting a character via SPI, with SS line control.
 *
 * @param SS_PORTx Slave select PORTx register
 * @param SS_PORTxn Slave select PORTxn register
 * @param SSmode choose if data is transmitted when pulling SS low (default) or when pulling SS high.
 * This is usefull when inverting schmitt triggers are used for SS line controll on master side.
 * @param c character that is going to be sent via SPI
 */
void SPI_transmitChar(volatile uint8_t *SS_PORTx, uint8_t SS_PORTxn, uint8_t SSmode, char c);

/**
 * Function for transmitting a string via SPI, with SS line control
 *
 * @param SS_PORTx Slave select PORTx register
 * @param SS_PORTxn Slave select PORTxn register
 * @param SSmode choose if data is transmitted when pulling SS low (default) or when pulling SS high.
 * This is usefull when inverting schmitt triggers are used for SS line controll on master side.
 * @param c char pointer that pints to an array element (string)
 */
void SPI_transmitString(volatile uint8_t *SS_PORTx, uint8_t SS_PORTxn, uint8_t SSmode, char *c);

/**
 * Function that reads a char from SPDR, with SS line control
 *
 * @param SS_PORTxSlave select PORTx register
 * @param SS_PORTxnSlave select PORTxn register
 * @param SSmode choose if data is transmitted when pulling SS low (default) or when pulling SS high.
 * This is usefull when inverting schmitt triggers are used for SS line controll on master side.
 * @return A char that is read from SPDR register
 */
char SPI_receiveChar(volatile uint8_t *SS_PORTx, uint8_t SS_PORTxn, uint8_t SSmode);

/**
 * Function for converting an array of hex values to single hex value.
 * Takes an array that stores individual hex values and returns combined uint64_t
 * hex value from all array elements.
 *
 * @param array array of hex values that are going to be combined
 * @return uint64_t value that represents all hex values combined from an array
 */
uint64_t hexArrayToHex(char array[]);

/**
 * Function for transmitting a hex number via SPI, with SS line control.
 ** For numBytes parameter, it is recommended to define a custom value called [HEX_DATA_BYTES].
 ** By defining [HEX_DATA_BYTES] a compiler warning is avoided, or else it is set to 8.
 *! [HEX_DATA_BYTES] has to be less or equal to 8!
 *
 * @param SS_PORTx Slave select PORTx register
 * @param SS_PORTxn Slave select PORTxn register
 * @param SSmode choose if data is transmitted when pulling SS low (default) or when pulling SS high.
 * This is usefull when inverting schmitt triggers are used for SS line controll on master side.
 * @param numBytes number of hex bytes that are going to be sent via SPI.
 * @param hexNumber hex number that is going to be sent via SPI
 */
void SPI_transmitHex(volatile uint8_t *SS_PORTx, uint8_t SS_PORTxn, uint8_t SSmode, uint8_t numBytes, uint64_t hexNumber);

#endif