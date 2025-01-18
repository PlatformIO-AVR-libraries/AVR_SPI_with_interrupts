/**
 * @file AVR_SPI_with_interrupts.h
 * @author Lukas Ternjej
 *
 * Header file that is going to be included in main project file
 * Defines all necessary functions, constants and variables.
 *
 * @date 2025-01-19
 */

#ifndef AVR_SPI_WITH_INTERRUPTS_H_
#define AVR_SPI_WITH_INTERRUPTS_H_

#include <avr/interrupt.h>
#include <avr/io.h>
#include <stdbool.h>
#include <stdlib.h>
#include <string.h>
#include <util/delay.h>

#include "AVR_SPI_char_defines.h"
#include "AVR_SPI_pin_defines.h"

// bit order
#define LSB_FIRST 0x20     // start data transmission with least significant bit first
#define MSB_FIRST 0x00     // start data transmission with most significant bit first

// Master/slave mode
#define MASTER_MODE 1     // SPI device in master mode
#define SLAVE_MODE  0     // SPI device in slave mode

// SPI mode
#define SPI_MODE_0 0x00     // clock logic low when idle, data sampled on rising edge, shifted out on falling edge
#define SPI_MODE_1 0x04     // clock logic low when idle, data sampled on falling edge, shifted out on rising edge
#define SPI_MODE_2 0x08     // clock logic high when idle, data sampled on rising edge, shifted out on falling edge
#define SPI_MODE_3 0x0C     // clock logic high when idle, data sampled on falling edge, shifted out on rising edge

// mask for setting SPI fosc
#define FOSC_MASK 0x03     // 0b00000011

// SPI clock rate
#define FOSC_DIV4   0x00     // FCPU/4, in binary form 0b00000000
#define FOSC_DIV16  0x01     // FCPU/16, in binary form 0b00000001
#define FOSC_DIV64  0x02     // FCPU/64, in binary form 0b00000010
#define FOSC_DIV128 0x03     // FCPU/128, in binary form 0b00000011

// these SPI clock rates require setting SPI2X bit in SPSR register and specific bits in SPCR
#define FOSC_DIV2  0x04     // FCPU/2, in binary form 0b00000100
#define FOSC_DIV8  0x05     // FCPU/8, in binary form 0b00000101
#define FOSC_DIV32 0x06     // FCPU/32, in binary form 0b00000110

// default or inverted SS line controll
#define INVERTED_SS_CONTROL 0
#define DEFAULT_SS_CONTROL  1

/**
 * Function for initializing SPI communication on Atmel AVR 8-bit microcontrollers that have a dedicated SPI module.
 ** This function doesn't handle multiple slave devices; manual control of multiple SS lines is mandatory.
 *! Make sure that SPI clock frequency that master generates is less than slave device F_CPU/4!!!!
 *
 * @param deviceMode master or slave mode
 * @param dataOrder least or most significant bit first
 * @param SPI_mode SPI mode 0, 1, 2 or 3
 * @param clockRate master SPI clock rate
 */
void SPI_init(uint8_t deviceMode, uint8_t dataOrder, uint8_t SPIMode, uint8_t clockRate);

/**
 * Function that returns an uint8_t from master SPDR register.
 * Write dummy data to SPDR register to generate SCK for transmission.
 *
 * @return uint8_t from SPDR register
 */
uint8_t SPI_masterReadUint8_t();

/**
 * Function that returns an uint8_t from SPDR register.
 *
 * @return uint8_t from SPDR register
 */
uint8_t SPI_readUint8_t();

/**
 * Function that sets all array elements to '\0'.
 *
 * @param array array to be flushed
 * @param size number of array elements
 */
void flushBuffer(uint8_t array[], size_t size);

/**
 * Function that compares two strings. First string consists of uint8_t characters, and the second string consists of char characters.
 *
 * @param str1 string of uint8_t characters
 * @param str2 string of char characters
 * @return 0 if strings are equal, >0 if the first non-matching character in str1 is greater (in ASCII) than that of str2,
 * <0 if the first non-matching character in str1 is lower (in ASCII) than that of str2
 */
int SPI_strcmp(uint8_t *str1, char *str2);

/**
 * Function that checks if SPI data transmission is complete.
 *
 * @return true if [STRING_END_CHAR] is reached; else, return false
 */
bool SPI_readAll(void);

/**
 * Function that writes an uint8_t in SPDR register. When in master mode,
 * writing to the SPDR register generates SPI clock.
 *
 * @param data uint8_t that is going to be written to SPDR register
 */
void SPI_masterPutUint8_t(uint8_t data);

/**
 * Writes an uint8_t to SPDR register.
 *
 * @param data uint8_t that is going to be written to SPDR register
 */
void SPI_putUint8_t(uint8_t data);

/**
 * Function for transmitting an uint8_t via SPI, with SS line control.
 *
 * @param SS_PORTx Slave select PORTx register
 * @param SS_PORTxn Slave select PORTxn register
 * @param SSmode choose if data is transmitted when pulling SS low (default) or when pulling SS high.
 * This is usefull when inverting schmitt triggers are used for SS line controll on master side.
 * @param data uint8_t that is going to be transmitted via SPI
 */
void SPI_transmitUint8_t(volatile uint8_t *SS_PORTx, uint8_t SS_PORTxn, uint8_t SSmode, uint8_t data);

/**
 * Function for transmitting a string of chars via SPI, with SS line control.
 *
 * @param SS_PORTx Slave select PORTx register
 * @param SS_PORTxn Slave select PORTxn register
 * @param SSmode choose if data is transmitted when pulling SS low (default) or when pulling SS high.
 * This is usefull when inverting schmitt triggers are used for SS line controll on master side.
 * @param data char pointer that pints to an array element (string), for transmissio via SPI
 */
void SPI_transmitString(volatile uint8_t *SS_PORTx, uint8_t SS_PORTxn, uint8_t SSmode, char *data);

/**
 * FFunction that reads an uint8_t from SPDR, with SS line control.
 *
 * @param SS_PORTx select PORTx register
 * @param SS_PORTxn select PORTxn register
 * @param SSmode choose if data is transmitted when pulling SS low (default) or when pulling SS high.
 * This is usefull when inverting schmitt triggers are used for SS line controll on master side.
 * @return A uint8_t that is read from SPDR register
 */
uint8_t SPI_receiveUint8_t(volatile uint8_t *SS_PORTx, uint8_t SS_PORTxn, uint8_t SSmode);

/**
 * Takes an array that stores individual uint8_t values and returns combined uint64_t
 * value from all array elements.
 *
 * @param array array of hex values that are going to be combined
 * @param size number of array elements
 * @return uint64_t value that represents all hex values combined from an array
 */
uint64_t hexArrayToUint64_t(uint8_t array[], size_t size);

/**
 * Function for transmitting a hex number via SPI, with SS line control.
 ** For numBytes parameter, it is recommended to define a custom value called [HEX_DATA_BYTES].
 *! [HEX_DATA_BYTES] has to be less or equal to 8!
 *
 * @param SS_PORTx Slave select PORTx register
 * @param SS_PORTxn Slave select PORTxn register
 * @param SSmode choose if data is transmitted when pulling SS low (default) or when pulling SS high.
 * This is usefull when inverting schmitt triggers are used for SS line controll on master side.
 * @param numBytes number of hex bytes that are going to be sent via SPI.
 * @param hexNumber hex number that is going to be transmitted via SPI
 */
void SPI_transmitHex(volatile uint8_t *SS_PORTx, uint8_t SS_PORTxn, uint8_t SSmode, uint8_t numBytes, uint64_t hexNumber);

#endif