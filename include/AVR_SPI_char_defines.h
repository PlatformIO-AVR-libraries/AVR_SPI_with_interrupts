/**
 * @file AVR_SPI_char_defines.h
 * @author Lukas Ternjej
 *
 * Header file for defining control characters for SPI library.
 *
 * @date 2024-03-06
 */

#ifndef AVR_SPI_CHAR_DEFINES_H_
#define AVR_SPI_CHAR_DEFINES_H_

#define DATA_END_CHAR 0x0D                // String message end character, 0x0D is carriage return (CR)
#define DATA_LENGTH   50 + 1              // Mximum data in a message + end character

extern uint8_t SPI_data[DATA_LENGTH];     // Array for storing incoming SPI data

#endif