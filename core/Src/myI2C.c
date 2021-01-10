/*
 * myI2C.c
 *
 *  Created on: Jan 8, 2021
 *      Author: Wiktor Lechowicz
 */

#include "myI2C.h"

void myI2C_Init()
{
    // TODO: add I2C1 ( SENSORS)

    // GPIOB CLK enable
    RCC->APB2ENR |= RCC_APB2ENR_IOPBEN;

    // DMA1 CLK enable
    RCC->AHBENR |= RCC_AHBENR_DMA1EN;
    HAL_NVIC_SetPriority(DMA1_Channel4_IRQn, 0, 0);
    HAL_NVIC_EnableIRQ(DMA1_Channel4_IRQn);                                             // enable DMA CH4 interrupt to inform about end of transfer
    // I2C2 //


    // PB11 and PB12 configuration for use with I2C2
    GPIOB->CRH |= (0x3UL << GPIO_CRH_MODE11_Pos) | (0x3UL << GPIO_CRH_MODE10_Pos);      // PB11 and PB10 as fast mode outputs
    GPIOB->CRH |= (0x3UL << GPIO_CRH_CNF11_Pos) | (0x3UL << GPIO_CRH_CNF10_Pos);        // PB11 and PB10 as alternate function Open-Drain pins


    // I2C2 reset
    RCC->APB1ENR |= RCC_APB1ENR_I2C2EN;         // I2C2 CLK enable
    I2C2->CR1 |= I2C_CR1_SWRST;
    I2C2->CR1 &= !I2C_CR1_SWRST;                // reset I2C2

    // I2C2 configuration
    I2C2->CR2 |= I2C_CR2_FREQ & 36;             // for APB1 frequency = 36Mhz
    I2C2->CCR |= I2C_CCR_FS;                    // fast mode
    I2C2->CCR |= 27;                           // change it to 27 for 400khz
    I2C2->TRISE |= 0x02;
    I2C2->CR1 |= I2C_CR1_PE;                    // enable I2C2


    // DMA CH4 is configured at the start of myI2C_writeByteStreamDMA() function
}

uint8_t myI2C_writeByte(I2C_TypeDef * I2Cx, uint8_t slaveAddr, uint8_t memAddr, uint8_t byte)
{
    uint32_t reg = 0;

    while(I2Cx->SR2 & I2C_SR2_BUSY);                                    // wait until I2Cx not busy

    I2Cx->CR1 |= I2C_CR1_START;                                         // generate start condition
    while(!(I2Cx->SR1 & I2C_SR1_SB));                                   // wait until start flag SB is set

    I2Cx->DR = slaveAddr & 0xFE;                                        // write slave address to data register

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

uint8_t myI2C_writeByteStreamDMA(I2C_TypeDef * I2Cx, uint8_t slaveAddr, uint8_t memAddr, uint8_t * data, uint16_t dataLen)
{
    uint32_t i = 0;
    // DMA handle for I2C2 (OLED)
    if(I2Cx == I2C2)
    {
        // reset DMA CH4 configuration and initialize it
        DMA1_Channel4->CCR &= ~DMA_CCR_EN;                                  // turn off DMA1 CH4
        while(DMA1_Channel4->CCR & DMA_CCR_EN);
        DMA1->IFCR |= 0x0FFFFF;                                             // clear interrupt flags

        DMA1_Channel4->CPAR = (uint32_t)&(I2C2->DR);                         // destination periph register
        DMA1_Channel4->CMAR = (uint32_t)data;                               // source address
        DMA1_Channel4->CNDTR = dataLen;                                     // data length
        DMA1_Channel4->CCR &= ~DMA_CCR_PL_Pos;                              // medium DMA priority
        DMA1_Channel4->CCR |= 0b01 << DMA_CCR_PL_Pos;
        DMA1_Channel4->CCR |= DMA_CCR_MINC;                                 // memory increment mode
        DMA1_Channel4->CCR &= ~DMA_CCR_PINC;
        DMA1_Channel4->CCR |= DMA_CCR_DIR;                                  // memory to peripherial direction
        DMA1_Channel4->CCR |= DMA_CCR_TEIE;                                 // enable transfer error interrupt

        DMA1_Channel4->CCR |= DMA_CCR_EN;                                   // enable DMA CH4

        // enable DMA IT
        DMA1_Channel4->CCR |= DMA_CCR_TCIE;


        // send addresses in polling mode
        while(I2Cx->SR2 & I2C_SR2_BUSY);                                    // wait until I2Cx not busy

        I2Cx->CR1 |= I2C_CR1_START;                                         // generate start condition
        while(!(I2Cx->SR1 & I2C_SR1_SB));                                   // wait until start flag SB is set

        I2Cx->DR = slaveAddr;                                               // write slave address to data register

        while(!(I2Cx->SR1 & I2C_SR1_ADDR));                                 // wait until address sent

        i = I2Cx->SR2;

        I2Cx->DR = memAddr;                                                 // write memory address

        while( !(I2Cx->SR1 & I2C_SR1_TXE) );                                // wait until DR empty

        i = I2Cx->SR2;                                                      // clear address flag

        // set DMAEN bit
        I2Cx->CR2 |= I2C_CR2_DMAEN;

        if( (I2Cx->SR1 & (I2C_SR1_AF | I2C_SR1_ARLO | I2C_SR1_BERR) ))      // check for errors
        {
            I2Cx->SR1 &= ~(I2C_SR1_AF | I2C_SR1_ARLO | I2C_SR1_BERR);
            return myI2C_FAILURE;
        }

        // end of I2C2 transfer will be handled in DMA_CH4 IRQN once DMA transfer completes.
        return myI2C_SUCCESS;
    } else {
        return myI2C_FAILURE;
    }
}
