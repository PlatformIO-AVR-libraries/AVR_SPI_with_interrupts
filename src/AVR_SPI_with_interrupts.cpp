/**
 * @file AVR_SPI_with_interrupts.cpp
 * @author Lukas Ternjej
 *
 * Interrupt driven SPI library .cpp file
 *
 * @date 2024-03-06
 */

#include "AVR_SPI_with_interrupts.h"

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
void SPI_init(uint8_t deviceMode, uint8_t dataOrder, uint8_t SPIMode, uint8_t clockRate)
{
    if(deviceMode == MASTER_MODE)
    {
        // set SS, MOSI, SCK as output
        SPI_DDRx |= (1 << SS_PIN_PORTxn) | (1 << MOSI_PIN_PORTxn) | (1 << SCK_PIN_PORTxn);
        SPI_DDRx &= ~(1 << MISO_PIN_PORTxn);     // set MISO pin as input
        SPCR |= (1 << MSTR);                     // set device SPI in master mode

        // these clock rates requre setting bits in both SPCR and SPSR registers
        if((clockRate == FOSC_DIV2) || (clockRate == FOSC_DIV8) || (clockRate == FOSC_DIV32))
            SPSR |= (1 << SPI2X);     // FOSC_DIV2, FOSC_DIV8, FOSC_DIV32 require setting bit SPI2X in SPSR

        SPCR |= clockRate;            // set clock rate
    }

    else
    {
        // set SS, MOSI, SCK as input
        SPI_DDRx &= ~((1 << SS_PIN_PORTxn) | (1 << MOSI_PIN_PORTxn) | (1 << SCK_PIN_PORTxn));
        SPI_DDRx |= (1 << MISO_PIN_PORTxn);     // set MISO pin as output
        SPCR &= ~(1 << MSTR);                   // set device SPI in slave mode
        SPCR |= (1 << SPIE);                    // enable SPI interrupt flag
        // slave doesn't care about clock rate
    }

    SPCR |= dataOrder | SPIMode | (1 << SPE);     // set LSB or MSB first, SPI mode and enable SPI
}

/**
 * Function that returns a character from master SPDR register
 * Write dummy data to SPDR register to generate SCK for transmission
 *
 * @return returns a char from SPSR register
 */
char SPI_masterReadChar()
{
    SPDR = 0xFF;     // writing to SPDR generates SCK for transmission, write dummy data in the SPDR register

    while(!(SPSR & (1 << SPIF)))
        ;

    return SPDR;
}

/**
 * Function that returns a character from master SPDR register
 *
 * @return returns a char from SPDR register
 */
char SPI_readChar()
{
    while(!(SPSR & (1 << SPIF)))
        ;

    return SPDR;
}

char SPI_data[DATA_LENGTH] = {'\0'};
volatile char SPI_buffer[DATA_LENGTH];
volatile uint8_t dataIndex = 0;
volatile bool dataReceived = false;

// read SPI data in ISR routine
ISR(SPI_STC_vect)
{
    SPI_buffer[dataIndex] = SPDR;

    if(SPI_buffer[dataIndex++] == DATA_END_CHAR)
    {
        dataReceived = true;
        dataIndex = 0;
    }
}

/**
 * Function that flushes a buffer and sets all array elements to '\0'
 *
 * @param array array to be flushed
 */
void flushBuffer(char array[])
{
    uint8_t size = strlen(array);

    for(int i = 0; i < size; i++)
        array[i] = '\0';
}

/**
 * Function that checks if SPI data transmiossion is complete.
 *
 * @return Returns true if [STRING_END_CHAR] is reached, else return false
 */
bool SPI_readAll()
{
    if(dataReceived == true)
    {
        // flush SPI_data[] from previous data before reading next message
        flushBuffer(SPI_data);

        int i = 0;

        // read new data into SPI_data
        while(SPI_buffer[i] != DATA_END_CHAR)
        {
            SPI_data[i] = SPI_buffer[i];
            i++;
        }

        // clear volatile array and set all array elements to '\0'
        for(int i = 0; i < DATA_LENGTH; i++)
            SPI_buffer[i] = '\0';

        dataReceived = false;

        return true;
    }

    else
        return false;
}

/**
 * Function that writes a char in SPDR register. When in master mode,
 * writing to the SPDR register generates SPI clock.
 *
 * @param c character that is going to be written to SPDR register
 */
void SPI_masterPutChar(char c)
{
    SPDR = c;     // write data to SPI data register

    while(!(SPSR & (1 << SPIF)))
        ;         // wait till transmission complete
}

/**
 * Writes a character to SPDR register
 *
 * @param c character that is going to be written to SPDR register
 */
void SPI_putChar(char c)
{
    // Wait for empty transmit buffer
    while(!(SPSR & (1 << SPIF)))
        ;

    // Put data into buffer
    SPDR = c;
}

/**
 * Function for transmitting a character via SPI, with SS line control.
 *
 * @param SS_PORTx Slave select PORTx register
 * @param SS_PORTxn Slave select PORTxn register
 * @param SSmode choose if data is transmitted when pulling SS low (default) or when pulling SS high.
 * This is usefull when inverting schmitt triggers are used for SS line controll on master side.
 * @param c character that is going to be sent via SPI
 */
void SPI_transmitChar(volatile uint8_t *SS_PORTx, uint8_t SS_PORTxn, uint8_t SSmode, char c)
{
    uint8_t pullHigh = (*SS_PORTx) | (1 << SS_PORTxn);
    uint8_t pullLow = (*SS_PORTx) & ~(1 << SS_PORTxn);
    // in default mode pull SS pin low to start transmision
    // in inverted mode pull SS pin high to start transmision
    *SS_PORTx = (SSmode == DEFAULT_SS_CONTROL) * pullLow + (SSmode == INVERTED_SS_CONTROL) * pullHigh;

    SPI_masterPutChar(c);                 // write data to SPDR register
    SPI_masterPutChar(DATA_END_CHAR);     // terminate with [DATA_END_CHAR]

    *SS_PORTx = (SSmode == DEFAULT_SS_CONTROL) * pullHigh + (SSmode == INVERTED_SS_CONTROL) * pullLow;
    // in default mode pull SS pin high to end transmision
    // in inverted mode pull SS pin low to end transmision
}

/**
 * Function for transmitting a string via SPI, with SS line control
 *
 * @param SS_PORTx Slave select PORTx register
 * @param SS_PORTxn Slave select PORTxn register
 * @param SSmode choose if data is transmitted when pulling SS low (default) or when pulling SS high.
 * This is usefull when inverting schmitt triggers are used for SS line controll on master side.
 * @param c char pointer that pints to an array element (string)
 */
void SPI_transmitString(volatile uint8_t *SS_PORTx, uint8_t SS_PORTxn, uint8_t SSmode, char *c)
{
    uint8_t pullHigh = (*SS_PORTx) | (1 << SS_PORTxn);
    uint8_t pullLow = (*SS_PORTx) & ~(1 << SS_PORTxn);
    // in default mode pull SS pin low to start transmision
    // in inverted mode pull SS pin high to start transmision
    *SS_PORTx = (SSmode == DEFAULT_SS_CONTROL) * pullLow + (SSmode == INVERTED_SS_CONTROL) * pullHigh;

    while(*c)
    {
        SPI_masterPutChar(*c);     // write data to SPDR register
        c++;
    }

    SPI_masterPutChar(DATA_END_CHAR);     // terminate with [DATA_END_CHAR]

    *SS_PORTx = (SSmode == DEFAULT_SS_CONTROL) * pullHigh + (SSmode == INVERTED_SS_CONTROL) * pullLow;
    // in default mode pull SS pin high to end transmision
    // in inverted mode pull SS pin low to end transmision
}

/**
 * Function that reads a char from SPDR, with SS line control
 *
 * @param SS_PORTxSlave select PORTx register
 * @param SS_PORTxnSlave select PORTxn register
 * @param SSmode choose if data is transmitted when pulling SS low (default) or when pulling SS high.
 * This is usefull when inverting schmitt triggers are used for SS line controll on master side.
 * @return A char that is read from SPDR register
 */
char SPI_receiveChar(volatile uint8_t *SS_PORTx, uint8_t SS_PORTxn, uint8_t SSmode)
{
    uint8_t pullHigh = (*SS_PORTx) | (1 << SS_PORTxn);
    uint8_t pullLow = (*SS_PORTx) & ~(1 << SS_PORTxn);
    // in default mode pull SS pin low to start transmision
    // in inverted mode pull SS pin high to start transmision
    *SS_PORTx = (SSmode == DEFAULT_SS_CONTROL) * pullLow + (SSmode == INVERTED_SS_CONTROL) * pullHigh;

    char c = SPI_masterReadChar();     // read data from SPDR register

    *SS_PORTx = (SSmode == DEFAULT_SS_CONTROL) * pullHigh + (SSmode == INVERTED_SS_CONTROL) * pullLow;
    // in default mode pull SS pin high to end transmision
    // in inverted mode pull SS pin low to end transmision

    return c;
}

/**
 * Function for converting an array of hex values to single hex value.
 * Takes an array that stores individual hex values and returns combined uint64_t
 * hex value from all array elements.
 *
 * @param array array of hex values that are going to be combined
 * @return uint64_t value that represents all hex values combined from an array
 */
uint64_t hexArrayToHex(char array[])
{
    uint8_t size = strlen(array);
    uint64_t combinedHex = 0;

    for(int i = 0; i < size; i++)
        combinedHex = (combinedHex << 8) | array[i];

    return combinedHex;
}

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
void SPI_transmitHex(volatile uint8_t *SS_PORTx, uint8_t SS_PORTxn, uint8_t SSmode, uint8_t numBytes, uint64_t hexNumber)
{
    uint8_t mask = 0xFF;

    uint8_t pullHigh = (*SS_PORTx) | (1 << SS_PORTxn);
    uint8_t pullLow = (*SS_PORTx) & ~(1 << SS_PORTxn);
    // in default mode pull SS pin low to start transmision
    // in inverted mode pull SS pin high to start transmision
    *SS_PORTx = (SSmode == DEFAULT_SS_CONTROL) * pullLow + (SSmode == INVERTED_SS_CONTROL) * pullHigh;

    for(int i = numBytes - 1; i >= 0; i--)
        SPI_masterPutChar((hexNumber >> (i * 8)) & mask);     // Send each byte of the hexadecimal number

    SPI_masterPutChar(DATA_END_CHAR);                         // terminate with [DATA_END_CHAR]

    *SS_PORTx = (SSmode == DEFAULT_SS_CONTROL) * pullHigh + (SSmode == INVERTED_SS_CONTROL) * pullLow;
    // in default mode pull SS pin high to end transmision
    // in inverted mode pull SS pin low to end transmision
}