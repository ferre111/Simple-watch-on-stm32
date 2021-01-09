/*
 * myI2C.c
 *
 *  Created on: Jan 8, 2021
 *      Author: Wiktor Lechowicz
 */

#include "myI2C.h"

void myI2C_Init()
{
    // GPIOs for I2C2 //

    RCC->APB2ENR |= RCC_APB2ENR_IOPBEN;                 // enable GPIOB clock

    // configure PB11 and PB 12 pins for use with I2C2  //

    GPIOB->CRH |= (0x3UL << GPIO_CRH_MODE11_Pos) | (0x3UL << GPIO_CRH_MODE10_Pos);      // PB11 and PB10 as fast mode outputs
    GPIOB->CRH |= (0x3UL << GPIO_CRH_CNF11_Pos) | (0x3UL << GPIO_CRH_CNF10_Pos);        // PB11 and PB10 as alternate function Open-Drain pins


    // I2C2 //
    RCC->APB1ENR |= RCC_APB1ENR_I2C2EN;            // enable I2C2 clock
    I2C2->CR1 |= I2C_CR1_SWRST;
    I2C2->CR1 &= !I2C_CR1_SWRST;                // reset I2C2

    I2C2->CR2 |= I2C_CR2_FREQ & 0x22;           // for APB1 frequency = 36Mhz

    I2C2->CCR |= I2C_CCR_FS;                    // fast mode
                                                    // slow mode
    I2C2->CCR |= 270;                          // change it to 27 for 400khz

    I2C2->TRISE |= 0x02;

    I2C2->CR1 |= I2C_CR1_PE;                    // enable I2C2
}


uint8_t myI2C_writeByte(I2C_TypeDef * I2Cx, uint8_t slaveAddr, uint8_t memAddr, uint8_t byte)
{
    uint32_t reg = 0;

    while(I2Cx->SR2 & I2C_SR2_BUSY);                                    // wait until I2Cx not busy

    I2Cx->CR1 |= I2C_CR1_START;                                         // generate start condition
    while(!(I2Cx->SR1 & I2C_SR1_SB));                                   // wait until start flag SB is set

    I2Cx->DR = slaveAddr;                                               // write slave address to data register

    while(!(I2Cx->SR1 & I2C_SR1_ADDR));                                 // wait until address sent

    reg = I2Cx->SR2;


    if( (I2Cx->SR1 & (I2C_SR1_AF | I2C_SR1_ARLO | I2C_SR1_BERR) ))      // check for errors
    {
        I2Cx->SR1 &= ~(I2C_SR1_AF | I2C_SR1_ARLO | I2C_SR1_BERR);
        return myI2C_FAILURE;
    }

    I2Cx->DR = memAddr;                                                 // write memory address

    while( !(I2Cx->SR1 & I2C_SR1_TXE) );                                // wait until DR empty
    I2Cx->DR = byte;                                                    // write byte to send

    while( !(I2Cx->SR1 & I2C_SR1_BTF) );                                // wait until byte sent
    I2Cx->CR1 |= I2C_CR1_STOP;                                          // program stop request

    if( (I2Cx->SR1 & (I2C_SR1_AF | I2C_SR1_ARLO | I2C_SR1_BERR) ))      // check for errors
    {
        I2Cx->SR1 &= ~(I2C_SR1_AF | I2C_SR1_ARLO | I2C_SR1_BERR);
        return myI2C_FAILURE;
    }

    return myI2C_SUCCESS;
}


uint8_t myI2C_writeByteStream(I2C_TypeDef * I2Cx, uint8_t slaveAddr, uint8_t memAddr, uint8_t * data, uint16_t dataLen)
{
    uint32_t i = 0;

    while(I2Cx->SR2 & I2C_SR2_BUSY);                                    // wait until I2Cx not busy

    I2Cx->CR1 |= I2C_CR1_START;                                         // generate start condition
    while(!(I2Cx->SR1 & I2C_SR1_SB));                                   // wait until start flag SB is set

    I2Cx->DR = slaveAddr;                                               // write slave address to data register

    while(!(I2Cx->SR1 & I2C_SR1_ADDR));                                 // wait until address sent

    i = I2Cx->SR2;


    if( (I2Cx->SR1 & (I2C_SR1_AF | I2C_SR1_ARLO | I2C_SR1_BERR) ))      // check for errors
    {
        I2Cx->SR1 &= ~(I2C_SR1_AF | I2C_SR1_ARLO | I2C_SR1_BERR);
        return myI2C_FAILURE;
    }

    I2Cx->DR = memAddr;                                                 // write memory address

    for(i = 0; i < dataLen; i++){
        while( !(I2Cx->SR1 & I2C_SR1_TXE) );                                // wait until DR empty
        I2Cx->DR = data[i];                                                    // write byte to send
    }

    while( !(I2Cx->SR1 & I2C_SR1_BTF) );                                // wait until byte sent
    I2Cx->CR1 |= I2C_CR1_STOP;                                          // program stop request

    if( (I2Cx->SR1 & (I2C_SR1_AF | I2C_SR1_ARLO | I2C_SR1_BERR) ))      // check for errors
    {
        I2Cx->SR1 &= ~(I2C_SR1_AF | I2C_SR1_ARLO | I2C_SR1_BERR);
        return myI2C_FAILURE;
    }

    return myI2C_SUCCESS;
}
