/**
 * TWI Master mode driver
 * \file DriverTWIMaster.h
 * \brief TWI Master mode driver
*/

#ifndef DRIVER_TWI_MASTER_H
#define DRIVER_TWI_MASTER_H

#include "hwconfig.h"
#include <avr/interrupt.h>

/**
 * \brief Initialize TWI Master module
*/
void DriverTWIMInit();


/**
 * \brief Write data block to specified slave address. Blocks until completion.
 * \param writeData :pointer to data block
 * \param bytesToWrite :number of bytes to write
 * \return true: transaction started, false: cannot start transaction
*/
uint8_t TWIMWrite(uint8_t address,uint8_t *writeData,uint8_t bytesToWrite); 


/**
 * \brief Read data block from specified slave address. Blocks until completion.
 * \param readData :pointer to data buffer
 * \param bytesToRead :number of bytes to read
 * \return true: transaction started, false: cannot start transaction
*/
uint8_t TWIMRead(uint8_t address,uint8_t *readData,uint8_t bytesToRead);

/**
 * \brief Perform a write + read transaction to a device with a specified address. Blocks until completion.
 * \param writeData :pointer to write data block
 * \param bytesToWrite :number of bytes to write
 * \param readData :pointer to data buffer
 * \param bytesToRead :number of bytes to read
 * \return true: transaction started, false: cannot start transaction
*/
uint8_t TWIMWriteRead(uint8_t address,uint8_t *writeData,uint8_t bytesToWrite,uint8_t *readData,uint8_t bytesToRead);


#endif 