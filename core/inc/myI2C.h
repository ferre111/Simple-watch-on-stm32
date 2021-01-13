/*
 * myI2C.h
 *
 *  Created on: Jan 8, 2021
 *      Author: Wiktor Lechowicz
 *      Description:
 *      This file contains functions for initialization and handling of I2C peripherials for stm32f1xx devices.
 *      Desired initialization values have been hardcoded in myI2C_Init function.
 *
 *      Connections:
 *      PB10 - I2C2 SCL
 *      PB11 - I2C2 SDA
 *
 *      PB6 - I2C1 SCL
 *      PB7 - I2C1 SDA
 */
#ifndef INC_MYI2C_H_
#define INC_MYI2C_H_

#include "stm32f103xb.h"                                // this header contain


// error codes for detecting transmission failure(NACK, arbitration lost or bus error)
#define myI2C_SUCCESS    0x01
#define myI2C_FAILURE    0x00
/**
 * @brief This function initializes the I2C1 and I2C2 peripherials
 */
void myI2C_Init();


/**
 * @brief write a single byte to given memory location of a slave in blocking mode
 * @param I2Cx - I2C1 or I2C2
 * @param slaveAddr   -   slave address
 * @param memAddr   -   memory address
 * @param byte  -   byte to send
 * @retval myI2C status (myI2C_SUCCES or myI2C_FAILURE)
 */
uint8_t myI2C_writeByte(I2C_TypeDef * I2Cx, uint8_t slaveAddr, uint8_t memAddr, uint8_t byte);

/**
 * @brief write a byte stream to given memory location of a slave in blocking mode
 * @param I2Cx      -       I2C1 or I2C2
 * @param slaveAdrr -       slave address
 * @param memAddr   -       memory address
 * @param data      -       pointer to data to send
 * @param dataLen   -       length of byte stream
 * @retval myI2C status (myI2C_SUCCES or myI2C_FAILURE)
 */
uint8_t myI2C_writeByteStream(I2C_TypeDef * I2Cx, uint8_t slaveAddr, uint8_t memAddr, uint8_t * data, uint16_t dataLen);

/**
 * @brief writa a byte stream to slave starting from given memory location in DMA mode. This function works only with I2C2(OLED).
 * @param I2Cx      -       I2C2
 * @param slaveAddr -       slave address
 * @param memAddr   -       memory address
 * @param data      -       pointer to data to send
 * @param dataLen   -       length of byte stream
 * @retval myI2C status (myI2C_SUCCES or myI2C_FAILURE)
 */
uint8_t myI2C_writeByteStreamDMA(I2C_TypeDef * I2Cx, uint8_t slaveAddr, uint8_t memAddr, uint8_t * data, uint16_t dataLen);

/**
 * @brief read byte stream from given slave memory location in polling mode.
 * @param I2Cx      -       I2C1 or I2C2
 * @param slaveAddr -       slave address
 * @param memAddr   -       memory address
 * @param data      -       address at which the received byte stream will be stored
 * @param dataLen   -       length of byte stream
 * @retval myI2C status (myI2C_SUCCES or myI2C_FAILURE)
 */
uint8_t myI2C_readByteStream(I2C_TypeDef * I2Cx, uint8_t slaveAddr, uint8_t memAddr, uint8_t * data, uint8_t dataLen);

/**
 * @brief read signle byte from given slave memory location in polling mode.
 * @param I2Cx      -       I2C1 or I2C2
 * @param slaveAddr -       slave address
 * @param memAddr   -       memory address
 * @param data      -       address at which the received byte will be stored
 * @retval myI2C status (myI2C_SUCCES or myI2C_FAILURE)
 */

#endif /* INC_MYI2C_H_ */
