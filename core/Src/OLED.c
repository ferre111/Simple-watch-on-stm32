/*
 * OLED.c
 *
 *  Created on: Nov 19, 2020
 *      Author: Wiktor Lechowicz
 */
#include "OLED.h"
#include "i2c.h"
#include "ascii_font.h"

//---------------------------------------------------------------------------------------
// DEFINES
#define OLED_ADDRESS 0b01111000

// Control byte options
#define OLED_CONTROL_BYTE_                  0b00000000
    #define _OLED_SINGLE_BYTE                   0b10000000
    #define _OLED_MULTIPLE_BYTES                0b00000000
    #define _OLED_COMMAND                       0b00000000
    #define _OLED_DATA                          0b01000000


// basic command set
#define	OLED_CMD_EntireDisplayOnPreserve            0xA4
#define	OLED_CMD_EntireDisplayOn                    0xA5
#define OLED_CMD_SetNotInversedDisplay              0xA6
#define OLED_CMD_SetInversedDisplay                 0xA7
#define OLED_CMD_SetDisplayON                       0xAF
#define OLED_CMD_SetDisplayOFF                      0xAE
#define OLED_CMD_EnableChargePumpDuringDisplay      0x8D

//---------------------------------------------------------------------------------------
// INIT BYTE STREAM

const uint8_t initSequence[33] = {
        OLED_CMD_SetDisplayOFF,
        0x20,                   // set memory addressing mode
        0x02,                   // page addressing
        //0x01,                 // vertical increment

        0x22,                   // set page addressing
        0x00,                   // start
        OLED_NUM_OF_PAGES - 1,  // stop

        0x21,                   // set column addressing
        0x00,                   // start
        OLED_X_SIZE - 1,        // stop

        0xB0,                   // set initial page 0

        0x00,                   // set initial column 0
        0x10,

        0xD5,                   // set display clock frequency and prescaler
        0x80,

        0xD9,                   // set pre-charge period
        0x22,

        0xA8,                   // set MUX ratio
        0x3F,

        0xD3,                   // set display offset
        0x00,

        0x40,                   // set display start line

        0xA1,                   // mirror horizontally
        // 0xA0

        0xC0,                   // set COM scan direction

        0xDA,                   // set com pins hw configuration
        0x12,

        0x81,                   // set contrast
        0x9F,

        0xC8,                   // mirror vertically
        //0xC0

        OLED_CMD_EntireDisplayOnPreserve,

        OLED_CMD_SetNotInversedDisplay,

        OLED_CMD_EnableChargePumpDuringDisplay,
        OLED_CMD_SetDisplayON,
        OLED_CMD_SetDisplayON,
};

//---------------------------------------------------------------------------------------
/* STATIC VARIABLES */
/*  display data buffer*/
//  7 pages of 128 8-bit columns
//   -> 0       1       2 3 4 ... 127
//  |  [byte]   [byte]  . . .     .
//  v  [byte]   .       .
//  0   .                 .
//  1   .
//  .
//  .
//  7   .

typedef struct
{
    uint8_t             firstBuffer[OLED_NUM_OF_PAGES*OLED_X_SIZE];
    uint8_t             secondBufffer[OLED_NUM_OF_PAGES*OLED_X_SIZE];
    uint8_t             addressArray[3];
    uint8_t *           currentBuffer;
    uint8_t             firstBufferAvailable : 1;

} oled_t;

oled_t oled;
//static uint8_t          buffer[OLED_NUM_OF_PAGES][OLED_X_SIZE];
//#static uint8_t          addressArray[3];
static uint8_t i, j, v, c;

//---------------------------------------------------------------------------------------
/* STATIC FUNCTIONS */
/*send single command to driver*/
static void sendCommand(uint8_t command)
{
    HAL_I2C_Mem_Write(&OLED_I2C_HANDLE, OLED_ADDRESS, 0x00, 1, &command, 1, HAL_MAX_DELAY);
}

/* send stream of commands to driver */
static void sendCommandStream(const uint8_t stream[], uint8_t streamLength)
{
    HAL_I2C_Mem_Write(&OLED_I2C_HANDLE, OLED_ADDRESS, 0x01, 1, stream, streamLength, HAL_MAX_DELAY);
}

/* set next column pointer in display driver */
static void setAddress(uint8_t page, uint8_t column)
{
    oled.addressArray[0] = 0xB0 | (page & 0x07);
    oled.addressArray[1] = column & 0x0F;
    oled.addressArray[2] = 0x10 | ((0xF0 & column) >> 4);

    HAL_I2C_Mem_Write(&OLED_I2C_HANDLE, OLED_ADDRESS, OLED_CONTROL_BYTE_ | _OLED_COMMAND | _OLED_MULTIPLE_BYTES, 1,
            oled.addressArray, 3, HAL_MAX_DELAY);
    //sendCommandStream(addressArray, 3);
}

/* write data to next column */
static void writeNextColumn(uint8_t data)
{
    HAL_I2C_Mem_Write(&OLED_I2C_HANDLE, OLED_ADDRESS, OLED_CONTROL_BYTE_ | _OLED_DATA | _OLED_SINGLE_BYTE,
            1, &data, 1, HAL_MAX_DELAY);
}


//---------------------------------------------------------------------------------------
/* API functions */

// OK
void OLED_Init()
{
    oled.currentBuffer = oled.firstBuffer;
    sendCommandStream(initSequence, 32);
    OLED_clearScreen();
    OLED_update();
}

// OK
void OLED_update()
{
    for(v = 0; v < OLED_NUM_OF_PAGES; v++){
        setAddress(v, 0);
        HAL_I2C_Mem_Write(&OLED_I2C_HANDLE, OLED_ADDRESS, OLED_CONTROL_BYTE_ | _OLED_DATA | _OLED_MULTIPLE_BYTES,
                1, (uint8_t * )(oled.currentBuffer + v*OLED_X_SIZE), OLED_X_SIZE, HAL_MAX_DELAY);
    }
}

// OK
void OLED_clearScreen()
{
    for(v = 0; v < OLED_NUM_OF_PAGES; v++){
        for(uint8_t c = 0; c < OLED_X_SIZE; c++){
            *(oled.currentBuffer + v*OLED_X_SIZE + c) = 0;
        }
    }
}

// OK
void OLED_printText(uint8_t verse, uint8_t column, char * text)
{
    i = 0;
    while(text[i] != '\0')
    {
        for(j = 0; j < 5; j++){
            *(oled.currentBuffer + verse*OLED_X_SIZE + column++) = font_ASCII[text[i] - ' '][j];
        }

        if(verse >= OLED_X_SIZE - 1)
            verse++;
        else
            //oled.currentBuffer[verse][column++] = 0x00;
            *(oled.currentBuffer + verse*OLED_X_SIZE + column++) = 0x00;

        i++;
    }
}


// OK
void OLED_drawLine(uint8_t x0, uint8_t y0, uint8_t x1, uint8_t y1)
{
    static float tan = 0.0f;
    static float oneOverTan = 0.0f;
    if(y1 == y0)
    {
        tan = 0.0f;
        oneOverTan = 999.0f;
    } else if(x1 == x0)
    {
        tan = 999.0f;
        oneOverTan = 0.0f;
    } else
    {
        float temp = ( (float) y1 - y0) / ( (float) x1 - x0);
        if(temp > 0)
            tan = temp;
        else tan = -temp;
        oneOverTan = 1.0f/tan;
    }

    int8_t xDir = 0, yDir = 0;
    if(x1 - x0 > 0)
        xDir = 1;
    else xDir = -1;

    if(y1 - y0 > 0)
        yDir = 1;
    else
        yDir = -1;

    float y = y0;
    float x = x0;

    uint8_t numOfIterations = 0, xLen = 0, yLen = 0;

    if(x1 - x0 > 0)
        xLen = x1 - x0;
    else
        xLen = x0 - x1;

    if(y1 - y0 > 0)
        yLen = y1 - y0;
    else
        yLen = y0 - y1;

    if(xLen > yLen)
        numOfIterations = xLen + 1;
    else
        numOfIterations = yLen + 1;

    float yTemp = 0, xTemp = 0;

    if(1)
    {
        for(i = 0; i < numOfIterations; i++)
        {
            //*oled.currentBuffer[ ( uint8_t ) y / 8 ][ ( uint8_t ) x ] |= 1 << ( (uint8_t) y % 8);
            *(oled.currentBuffer + (( uint8_t ) y / 8)*OLED_X_SIZE + ( uint8_t ) x ) |= 1 << ( (uint8_t) y % 8);
            yTemp += tan;
            if(yTemp >= 1)
            {
                if(yDir == 1)
                    y++;
                else
                    y--;
                yTemp = 0;
            }

            xTemp += oneOverTan;
            if(xTemp >= 1)
            {
                if(xDir == 1)
                    x++;
                else
                    x--;
                xTemp = 0;
            }
        }
    }
}

void OLED_setDisplayOn()
{
    sendCommand(OLED_CMD_SetDisplayON);
}

void OLED_setDisplayOff()
{
    sendCommand(OLED_CMD_SetDisplayOFF);
}

void OLED_setInversed(uint8_t tf)
{
    if(tf)
        sendCommand(OLED_CMD_SetInversedDisplay);
    else
        sendCommand(OLED_CMD_SetNotInversedDisplay);
}

void OLED_drawImage(uint8_t xPos, uint8_t yPos,const uint8_t image[])
{
    uint8_t numOfCols = image[0] % OLED_X_SIZE;
    uint8_t numOfVerses = image[1] % OLED_NUM_OF_PAGES;


    i = 2;
    uint8_t rem = yPos % 8;
    uint8_t startingVerse = yPos / 8;
    if(rem != 0)
    {
        numOfVerses++;
    }

    for(v = startingVerse; v < startingVerse + numOfVerses; v++)
    {
        if(v == startingVerse)
        {
            for(c = xPos; c < numOfCols + xPos; c++)
            {
                *(oled.currentBuffer + v*OLED_X_SIZE + c) = image[i++] << rem;
            }
        }
        else if (v != startingVerse + numOfVerses - 1 )
        {
            for(c = xPos; c < numOfCols + xPos; c++)
            {
                *(oled.currentBuffer + v*OLED_X_SIZE + c) = image[i - numOfCols] >> (8 - rem);
                *(oled.currentBuffer + v*OLED_X_SIZE + c) |= image[i++] << rem;
            }
        }
        else
        {
            for(c = xPos; c < numOfCols + xPos; c++)
            {
                *(oled.currentBuffer + v*OLED_X_SIZE + c) = image[i++ - numOfCols] >> (8 - rem);
            }
        }
    }
}

void OLED_drawRect(uint8_t x0, uint8_t y0, uint8_t x1, uint8_t y1, enum OLED_Color color)
{
    uint8_t rem0 = y0 % 8;
    uint8_t rem1 = y1 % 8;

    v = y0 / 8;
    c = x0;
    while(c <= x1)
    {
        if(color == WHITE)
            *(oled.currentBuffer + v*OLED_X_SIZE + c++) |= 0xFF << rem0;
        else
            *(oled.currentBuffer + v*OLED_X_SIZE + c++) &= ~(0xFF << rem0);
    }
    v++;

    while(v < y1 / 8)
    {
        c = x0;
        while(c <= x1)
        {
            if(color == WHITE)
                *(oled.currentBuffer + v*OLED_X_SIZE + c++) = 0xFF;
            else
                *(oled.currentBuffer + v*OLED_X_SIZE + c++) = 0x00;
        }
        v++;
    }

    if(y1 % 8)
    {
        c = x0;
        while(c <= x1)
        {
            if(color == WHITE)
                *(oled.currentBuffer + v*OLED_X_SIZE + c++) |= 0xFF >> ( 8 - rem0 );
            else
                *(oled.currentBuffer + v*OLED_X_SIZE + c++) &= ~(0xFF >> ( 8 - rem1 ));
        }
    }
}

