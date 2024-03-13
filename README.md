# AVR_SPI_with_interrupts


## Table of contents
* [Introduction](#introduction)
* [Compatability](#compatability)
* [First setup](#first-setup)
* [Data transmission](#data-transmission)
* [Library functions](#library-functions)
* [Notes](#notes)
* [TODO](#todo)


## Introduction
AVR_SPI_with_interrupts is a library that simplifies use of SPI interface for Microchip megaAVR 8-bit microcontrollers that have a dedicated SPI module. This library implements interrupt driven SPI communication.


## Compatability:
### Tested AVR GCC versions (with AVR-LIBC environment):
- AVR GCC 5.4.0
- AVR GCC 12.2.0

***Library may work with other compilers or libraries, but this has not been tested.***

### Tested microcontrollers:
- ATmega32
- ATmega88

***Library may work with any Microchip megaAVR 8-bit microcontroller, as long as it has a dedicated SPI module, but this has not been tested.***


## First setup
1. include `AVR_SPI_with_interrupts.h` and `avr/interrupt.h` into main project file:
    ```c
    #include <AVR_SPI_with_interrupts.h>
    #include <avr/interrupt.h>
    ```
2. setup SPI and enable global interrupts:
    ```c
    SPI_init(MASTER_MODE, MSB_FIRST, SPI_MODE_0, FOSC_DIV16);
    sei();
    ```


## Data transmission
### MASTER DEVICE - transmitting data:
1. master device pulls SS pin low to start transmission.
2. transmission takes place until all data is sent, terminated by `END_CHAR`.
3. master pulls SS pin high to end transmission.

### SLAVE DEVICE - receiving data:
1. slave device waits for master to pull its SS pin low to start reception.
   - all received data is terminated by `END_CHAR` and can be modified in the `AVR_SPI-char_defines.h` header file.
2. when a slave device receives a byte, the ISR routine is called, and data is stored in the `SPI_data[]` array.
3. if `END_CHAR` is reached, the function `SPI_readAll()` will return true, indicating that all data has been received.
4. received data can then be read from the `SPI_data[]` register in the main loop.
5. data reception is done when master pulls the SS pin high.

### MASTER DEVICE - receiving data:
1. master device pulls the SS pin low to start reception.
2. clock signal is provided for the slave device, during which master will read data from the slave.
3. master device pulls the SS pin high to end reception.

***Master device can currently only receive a single byte of data; see [[Note 1](#note-1)].***

### SLAVE DEVICE - transmitting data:
1. slave device prepares data and waits for master device to pull its SS pin low to start transmission.
2. master provides a clock signal for the slave to transmit data.
3. master reads data from the slave.
4. master device pulls the SS pin high to end reception.

***Library provides functions for inverse SS pin logic, see [[Note 2](#note-2)].***


## Library functions:

Function for initializing SPI communication on Atmel AVR 8-bit microcontrollers that have a dedicated SPI module. This function doesn't handle multiple slave devices; manual control of multiple SS lines is mandatory.
***Make sure that SPI clock frequency that master generates is less than slave device F\_CPU/4!!!!***

```c
void SPI_init(uint8_t deviceMode, uint8_t dataOrder, uint8_t SPIMode, uint8_t clockRate);
```

***Parameters:***
1. deviceMode:
   - `MASTER_MODE` - device operates as master
   - `SLAVE_MODE` - device operates as slave
2. dataOrder:
   - `LSB_FIRST` - start data transmission with least significant bit first
   - `MSB_FIRST` - start data transmission with most significant bit first
3. SPImode:
   - `SPI_MODE_0` - clock logic low when idle, data sampled on rising edge, shifted out on falling edge
   - `SPI_MODE_1` - clock logic low when idle, data sampled on falling edge, shifted out on rising edge
   - `SPI_MODE_2` - clock logic high when idle, data sampled on rising edge, shifted out on falling edge
   - `SPI_MODE_3` - clock logic high when idle, data sampled on falling edge, shifted out on rising edge
4. clockRate:
   - `FOSC_DIV2` - SPI clock frequency set to F_CPU / 2
   - `FOSC_DIV4` - SPI clock frequency set to F_CPU / 4
   - `FOSC_DIV8` - SPI clock frequency set to F_CPU / 8
   - `FOSC_DIV16` - SPI clock frequency set to F_CPU / 16
   - `FOSC_DIV32` - SPI clock frequency set to F_CPU / 32
   - `FOSC_DIV64` - SPI clock frequency set to F_CPU / 64
   - `FOSC_DIV128` - SPI clock frequency set to F_CPU / 128

-------------------------------------------------------------------------

Function that returns an uint8_t from master SPDR register.
Write dummy data to SPDR register to generate SCK for transmission.
This function is useful when reading data from slave device as master.
It uses default SS pins; see microcontroller datasheet.

```c
uint8_t SPI_masterReadUint8_t();
```
***returns:*** uint8_t from SPDR register

-------------------------------------------------------------------------

Function that returns an uint8_t from SPDR register

```c
uint8_t SPI_readUint8_t();
```

***returns:*** uint8_t from SPDR register

-------------------------------------------------------------------------

Function that sets all array elements to '\0'. This function is useful when flushing
data from an array of values, essentially making all future data '\0' terminated.

```c
void flushBuffer(uint8_t array[], size_t size);
```

***Parameters:***
1. array[] - array to be flushed
2. size - number of array elements

-------------------------------------------------------------------------

Function that compares two strings. First string consists of uint8_t characters, and the second string consists of char characters. Since all data that is received on slave device is stored in a register that is of the uint8_t type, data cannot be compared with the C standard function strcmp(). It is useful when comparing input data with a predefined string command.

```c
int SPI_strcmp(uint8_t *str1, char *str2);
```

***Parameters:***
1. str1 - string of uint8_t characters
2. str2 - string of char characters

***returns:*** 0 if strings are equal, >0 if the first non-matching character in str1 is greater (in ASCII) than that of str2, <0 if the first non-matching character in str1 is lower (in ASCII) than that of str2

-------------------------------------------------------------------------

Function that checks if SPI data transmission is complete. This function should be used
every time data is received. It assures that all data bytes are received.

```c
bool SPI_readAll(void);
```

***returns:*** true if `STRING_END_CHAR` is reached; else, return false

-------------------------------------------------------------------------

Function that writes an uint8_t in SPDR register. When in master mode,
writing to the SPDR register generates SPI clock. This function is useful when transmitting data to slave device as master. It uses default SS pins; see microcontroller datasheet.

```c
void SPI_masterPutUint8_t(uint8_t data);
```

***Parameters:***
1. data - uint8_t that is going to be written to SPDR register

-------------------------------------------------------------------------

Writes an uint8_t to SPDR register

```c
void SPI_putUint8_t(uint8_t data);
```

***Parameters:***
1. data - uint8_t that is going to be written to SPDR register

-------------------------------------------------------------------------

Function for transmitting an uint8_t via SPI, ***with SS line control***. Use this function to transmit data to slave as  master.

```c
void SPI_transmitUint8_t(volatile uint8_t *SS_PORTx, uint8_t SS_PORTxn, uint8_t SSmode, uint8_t data);
```

***Parameters:***
1. SS_PORTx - SS pin PORTx register
2. SS_PORTxn - SS pin PORTx register
3. SS_mode:
   - `INVERTED_SS_CONTROL` - transmission starts by pulling SS pin high, ends with pulling SS pin low
   - `DEFAULT_SS_CONTROL` - transmission starts by pulling SS pin low, ends with pulling SS pin high
4. data - uint8_t that is going to be transmitted via SPI
-------------------------------------------------------------------------

Function for transmitting a string of chars via SPI, ***with SS line control***. Use this function to transmit data to slave as  master.

```c
void SPI_transmitString(volatile uint8_t *SS_PORTx, uint8_t SS_PORTxn, uint8_t SSmode, char *data);
```

***Parameters:***
1. SS_PORTx - SS pin PORTx register
2. SS_PORTxn - SS pin PORTx register
3. SS_mode:
   - `INVERTED_SS_CONTROL` - transmission starts by pulling SS pin high, ends with pulling SS pin low
   - `DEFAULT_SS_CONTROL` - transmission starts by pulling SS pin low, ends with pulling SS pin high
4. data - char pointer that points to an array element (string) for transmission via SPI

-------------------------------------------------------------------------

Function that reads an uint8_t from SPDR, ***with SS line control***. This function is useful
when master needs to read data from slave device.

```c
uint8_t SPI_receiveUint8_t(volatile uint8_t *SS_PORTx, uint8_t SS_PORTxn, uint8_t SSmode);
```

***Parameters:***
1. SS_PORTx - SS pin PORTx register
2. SS_PORTxn - SS pin PORTx register
3. SS_mode:
   - `INVERTED_SS_CONTROL` - transmission starts by pulling SS pin high, ends with pulling SS pin low
   - `DEFAULT_SS_CONTROL` - transmission starts by pulling SS pin low, ends with pulling SS pin high

***returns:*** uint8_t that is read from SPDR register

-------------------------------------------------------------------------

Takes an array that stores individual uint8_t values and returns combined uint64_t
value from all array elements. When receiving hex values, individual bytes are stored in main SPI buffer array. Use this function to transform individual hex values in an array into a single hex value. This is useful since a switch case could be implemented on slave device for specific use cases depending on received data.

```c
uint64_t hexArrayToUint64_t(uint8_t array[], size_t size);
```

***Parameters:***
1. array[] - array of hex values that are going to be combined
2. size - number of array elements

***returns:*** uint64_t value that represents all hex values combined from an array

-------------------------------------------------------------------------

Function for transmitting a hex number via SPI, ***with SS line control***.
For numBytes parameter, it is recommended to define a custom value called `HEX_DATA_BYTES`.
***`HEX_DATA_BYTES` has to be less than or equal to 8!***. This is because using more than 8 bytes of hex values would result in an overflow when using `hexArrayToUint64_t()` function.

```c
void SPI_transmitHex(volatile uint8_t *SS_PORTx, uint8_t SS_PORTxn, uint8_t SSmode, uint8_t numBytes, uint64_t hexNumber);
```

***Parameters:***
1. SS_PORTx - SS pin PORTx register
2. SS_PORTxn - SS pin PORTx register
3. SS_mode:
   - `INVERTED_SS_CONTROL` - transmission starts by pulling SS pin high, ends with pulling SS pin low
   - `DEFAULT_SS_CONTROL` - transmission starts by pulling SS pin low, ends with pulling SS pin high
4. numBytes - number of hex bytes that are going to be sent via SPI
5. hexNumber - hex number that is going to be transmitted via SPI

-------------------------------------------------------------------------


## Notes:
### Note 1:
- multiple byte reception on master device has not yet been implemented

### Note 2:
- choose if data is transmitted when pulling SS low (default) or when pulling SS high.This is useful when inverting Schmitt triggers are used for SS line control on master side.


## TODO
- implement multiple byte data reception for master device