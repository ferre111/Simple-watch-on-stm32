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
#define OLED_ADDRESS                            0b01111000

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


// MACROS
#define GET_DECPART(x)(x - (int)x)
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


// === TEXT FIELD ===
typedef struct
{
    uint8_t             id;
    uint8_t             x0;                                             // upper left corner of text field
    uint8_t             verse;                                          // verse number of text field
    char *              text;                                           // pointer to character sequence
                                                                        // If it is different from zero it will be
                                                                        // updated in buffer and decremented so both firstBuffer
                                                                        // and secondBuffer becomes updated.
    uint8_t             isUsed : 1;                                     // 1 if text field is used
} textField_t;

// === LINE ===
typedef struct
{
    uint8_t             id;
    uint8_t             x0;                                              // start point of line x coordinate
    uint8_t             y0;                                              // start point of line y coordinate
    uint8_t             x1;                                              // end point of line x coordinate
    uint8_t             y1;                                              // end point of line y coordinate
    uint8_t             isUsed : 1;                                      // 1 if line is used
} line_t;

// === RECTANGLE ===
typedef struct
{
    uint8_t             id;
    uint8_t             x0;                                              // upper left corner x coordinate
    uint8_t             y0;                                              // upper left y coordinate
    uint8_t             width;                                           // rectangle width
    uint8_t             height;                                          // rectangle height
    uint8_t             isUsed : 1;                                      // 1 if rectangle is used
} rectangle_t;

// === IMAGE ===
typedef struct
{
    uint8_t id;
    uint8_t x0;                                                         // upper left corner x coordinate
    uint8_t y0;                                                         // upper left corner y coordinate
    uint8_t * imageArray;                                               // pointer to array with image representation
    uint8_t isUsed : 1;                                                 // 1 if image is used
} image_t;

typedef struct
{
    uint8_t             firstBuffer[OLED_NUM_OF_PAGES*OLED_X_SIZE];      // two buffers used alternately for updating display
    uint8_t             secondBufffer[OLED_NUM_OF_PAGES*OLED_X_SIZE];
    uint8_t             addressArray[3];                                 // temp array used to send GRAM address through I2C
    uint8_t *           currentBuffer;                                   // pointer to currently used buffer
    textField_t         textFields[OLED_MAX_TEXT_FIELDS_COUNT];          // text fields
    line_t              lines[OLED_MAX_LINES_COUNT];                     // drawable lines
    rectangle_t         rectangles[OLED_MAX_RECTANGLES_COUNT];           // drawable rectangles
    image_t             images[OLED_MAX_IMAGES_COUNT];                   // drawable images
    uint8_t             firstBufferAvailable : 1;

} oled_t;

oled_t oled;
//static uint8_t          buffer[OLED_NUM_OF_PAGES][OLED_X_SIZE];
//#static uint8_t          addressArray[3];


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

#define SET_PIXEL(x, y)(*oled.currentBuffer + x + OLED_X_SIZE*((uint8_t)(y / 8) )) |= 0x01 << y % 8;

static void setPixel(uint8_t x, uint8_t y)
{
    *(oled.currentBuffer + x + OLED_X_SIZE*( (uint8_t) (y / 8) )) |= 0x01 << y % 8;
}

static void printText(uint8_t verse, uint8_t column, char * text)
{
    uint8_t i = 0;
    while(text[i] != '\0')
    {
        for(uint8_t j = 0; j < 5; j++){
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

static void drawLine(uint8_t x0, uint8_t y0, uint8_t x1, uint8_t y1)
{
    static float tan = 0.0f;
    static float oneOverTan = 0.0f;
    if(y1 == y0)
    {
        tan = 0.0f;
        oneOverTan = 99999.0f;
    } else if(x1 == x0)
    {
        tan = 99999.0f;
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
        for(uint8_t i = 0; i < numOfIterations; i++)
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
                //yTemp = 0;
                yTemp = yTemp - (int)yTemp;
            }

            xTemp += oneOverTan;
            if(xTemp >= 1)
            {
                if(xDir == 1)
                    x++;
                else
                    x--;
                //Temp = 0;
                xTemp = xTemp - (int)xTemp;
            }
        }
    }
}

static void drawRect(uint8_t x0, uint8_t y0, uint8_t x1, uint8_t y1, enum OLED_Color color){
    uint8_t rem0 = y0 % 8;
    uint8_t rem1 = y1 % 8;

    uint8_t v = y0 / 8;
    uint8_t c = x0;


    while(c <= x1)
    {
        if(color == WHITE)
            if(y1 / 8 == y0 / 8)
                *(oled.currentBuffer + v*OLED_X_SIZE + c++) |= (0xFF << rem0) & (0xFF >> (8 - rem1));
            else
                *(oled.currentBuffer + v*OLED_X_SIZE + c++) |= 0xFF << rem0;
        else
            if(y1 / 8 == y0 /8)
                *(oled.currentBuffer + v*OLED_X_SIZE + c++) &= ~((0xFF << rem0) & (0xFF >> (8 - rem1)));
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

    if((y1 % 8) && (y1 / 8 != y0 / 8))
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

static void drawImage(uint8_t xPos, uint8_t yPos,const uint8_t image[])
{
    uint8_t numOfCols = image[0] % OLED_X_SIZE;
    uint8_t numOfVerses = image[1] % OLED_NUM_OF_PAGES;

    uint8_t v;
    uint8_t c;
    uint8_t i = 2;
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
                *(oled.currentBuffer + v*OLED_X_SIZE + c) |= image[i++] << rem;
            }
        }
        else if (v != startingVerse + numOfVerses - 1 )
        {
            for(c = xPos; c < numOfCols + xPos; c++)
            {
                *(oled.currentBuffer + v*OLED_X_SIZE + c) |= image[i - numOfCols] >> (8 - rem);
                *(oled.currentBuffer + v*OLED_X_SIZE + c) |= image[i++] << rem;
            }
        }
        else
        {
            for(c = xPos; c < numOfCols + xPos; c++)
            {
                *(oled.currentBuffer + v*OLED_X_SIZE + c) |= image[i++ - numOfCols] >> (8 - rem);
            }
        }
    }
}

//---------------------------------------------------------------------------------------
/* API functions */

// OK
void OLED_Init()
{
    uint8_t i = 0;
    oled.currentBuffer = oled.firstBuffer;
    for(i = 0; i < OLED_MAX_TEXT_FIELDS_COUNT; i++)
    {
        oled.textFields[i].isUsed = 0;
    }
    for(i = 0; i < OLED_MAX_LINES_COUNT; i++)
    {
        oled.lines[i].isUsed = 0;
    }
    sendCommandStream(initSequence, 32);
    OLED_clearScreen();
    OLED_update();
}

void OLED_update()
{
    OLED_clearScreen();
    // update text fields
    for(uint8_t i = 0; i < OLED_MAX_TEXT_FIELDS_COUNT; i++)
    {
        if( ( oled.textFields[i].isUsed == 1 ))
            printText(oled.textFields[i].verse, oled.textFields[i].x0, oled.textFields[i].text);
    }
    // update lines
    for(uint8_t i = 0; i < OLED_MAX_LINES_COUNT; i++)
    {
        if( ( oled.lines[i].isUsed == 1 ))
            drawLine(oled.lines[i].x0, oled.lines[i].y0, oled.lines[i].x1, oled.lines[i].y1);
    }
    // update rectangles
    for(uint8_t i = 0; i < OLED_MAX_RECTANGLES_COUNT; i++)
    {
        if( oled.rectangles[i].isUsed == 1 )
            drawRect(oled.rectangles[i].x0, oled.rectangles[i].y0, oled.rectangles[i].x0 + oled.rectangles[i].width,
                    oled.rectangles[i].y0 + oled.rectangles[i].height, WHITE);
    }
    // update images
    for(uint8_t i = 0; i < OLED_MAX_IMAGES_COUNT; i++)
    {
        if( oled.images[i].isUsed == 1 )
            drawImage(oled.images[i].x0, oled.images[i].y0, oled.images[i].imageArray);
    }

    // send buffer to OLED
    for(uint8_t v = 0; v < OLED_NUM_OF_PAGES; v++){
        setAddress(v, 0);
        HAL_I2C_Mem_Write(&OLED_I2C_HANDLE, OLED_ADDRESS, OLED_CONTROL_BYTE_ | _OLED_DATA | _OLED_MULTIPLE_BYTES,
                1, (uint8_t * )(oled.currentBuffer + v*OLED_X_SIZE), OLED_X_SIZE, HAL_MAX_DELAY);
    }
}

void OLED_clearScreen()
{
    for(uint8_t v = 0; v < OLED_NUM_OF_PAGES; v++){
        for(uint8_t c = 0; c < OLED_X_SIZE; c++){
            *(oled.currentBuffer + v*OLED_X_SIZE + c) = 0;
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



// === TEXT FIELD ===
void OLED_createTextField(uint8_t * id, uint8_t x0, uint8_t verse, char* text)
{
    uint8_t i = 0;                                               // starting id
    uint8_t c = 0;

    while(oled.textFields[c].isUsed)                    // check which textField is unused
    {
        c++;
        if(c >= OLED_MAX_TEXT_FIELDS_COUNT)
        {
            id = NULL;
            return;
        }
    }

    uint8_t temp;
    do
    {

        i++;
        temp = 1;
        for(uint8_t j = 0; j < OLED_MAX_TEXT_FIELDS_COUNT; j++)                          // loop in all textFields to get unique id
        {
            if(oled.textFields[j].id == i)
            {
                temp = 0;
                break;
            }
        }
    } while(temp != 1);                                 // if any of text field have got id == i then increment i and iterate again
    *id = i;
    oled.textFields[c].id = i;
    oled.textFields[c].isUsed = 1;
    oled.textFields[c].x0 = x0;
    oled.textFields[c].verse = verse;
    oled.textFields[c].text = text;
}

void OLED_textFieldSetText(uint8_t id, char * text)
{
    for(uint8_t i = 0; i < OLED_MAX_TEXT_FIELDS_COUNT; i++)
    {
        if(oled.textFields[i].id == id)
        {
            oled.textFields[i].text = text;
            return;
        }
    }
}

void OLED_textFieldSetPosition(uint8_t id, uint8_t x, uint8_t verse)
{
    for(uint8_t i = 0; i < OLED_MAX_TEXT_FIELDS_COUNT; i++)
    {
        if(oled.textFields[i].id == id)
        {
            oled.textFields[i].x0 = x;
            oled.textFields[i].verse = verse;
            return;
        }
    }
}

// === LINE ===
void OLED_createLine(uint8_t * id, uint8_t x0, uint8_t y0, uint8_t x1, uint8_t y1)
{
    uint8_t i = 0;                                               // starting id
    uint8_t c = 0;

    while(oled.lines[c].isUsed)                    // check which line is unused
    {
        c++;
        if(c >= OLED_MAX_LINES_COUNT)
        {
            id = NULL;
            return;
        }
    }

    uint8_t temp;
    do
    {

        i++;
        temp = 1;
        for(uint8_t j = 0; j < OLED_MAX_LINES_COUNT; j++)                          // loop in all lines to get unique id
        {
            if(oled.lines[j].id == i)
            {
                temp = 0;
                break;
            }
        }
    } while(temp != 1);                                 // if any line have got id == i then increment i and iterate again
    *id = i;
    oled.lines[c].id = i;
    oled.lines[c].isUsed = 1;
    oled.lines[c].x0 = x0;
    oled.lines[c].y0 = y0;
    oled.lines[c].x1 = x1;
    oled.lines[c].y1 = y1;
}

void OLED_lineMoveStart(uint8_t id, uint8_t x0, uint8_t y0)
{
    for(uint8_t i = 0; i < OLED_MAX_LINES_COUNT; i++)
    {
        if(oled.lines[i].id == id)
        {
            oled.lines[i].x0 = x0;
            oled.lines[i].y0 = y0;
            return;
        }
    }
}

void OLED_lineMoveEnd(uint8_t id, uint8_t x1, uint8_t y1)
{
    for(uint8_t i = 0; i < OLED_MAX_LINES_COUNT; i++)
    {
        if(oled.lines[i].id == id)
        {
            oled.lines[i].x1 = x1;
            oled.lines[i].y1 = y1;
            return;
        }
    }
}

// === RECTANGLE ===
void OLED_createRectangle(uint8_t * id,  uint8_t x0, uint8_t y0, uint8_t width, uint8_t height)
{
    uint8_t i = 0;                                               // starting id
    uint8_t c = 0;

    while(oled.rectangles[c].isUsed)                    // check which rectangle is unused
    {
        c++;
        if(c >= OLED_MAX_RECTANGLES_COUNT)
        {
            id = NULL;
            return;
        }
    }

    uint8_t temp;
    do
    {

        i++;
        temp = 1;
        for(uint8_t j = 0; j < OLED_MAX_RECTANGLES_COUNT; j++)                          // loop in all rectangels to get unique id
        {
            if(oled.rectangles[j].id == i)
            {
                temp = 0;
                break;
            }
        }
    } while(temp != 1);                                 // if any of rectangle have got id == i then increment i and iterate again
    *id = i;
    oled.rectangles[c].id = i;
    oled.rectangles[c].isUsed = 1;
    oled.rectangles[c].x0 = x0;
    oled.rectangles[c].y0 = y0;
    oled.rectangles[c].width = width;
    oled.rectangles[c].height = height;
}

void OLED_rectangleSetPosition(uint8_t id, uint8_t x0, uint8_t y0)
{
    for(uint8_t i = 0; i < OLED_MAX_RECTANGLES_COUNT; i++)
    {
        if(oled.rectangles[i].id == id)
        {
            oled.rectangles[i].x0 = x0;
            oled.rectangles[i].y0 = y0;
            return;
        }
    }
}

void OLED_rectangleSetDimensions(uint8_t id, uint8_t width, uint8_t height)
{
    for(uint8_t i = 0; i < OLED_MAX_RECTANGLES_COUNT; i++)
    {
        if(oled.rectangles[i].id == id)
        {
            oled.rectangles[i].width = width;
            oled.rectangles[i].height = height;
            return;
        }
    }
}

// === IMAGE ===
void OLED_createImage(uint8_t * id, uint8_t x0, uint8_t y0, uint8_t * imageArray)
{
    uint8_t i = 0;                                               // starting id
    uint8_t c = 0;

    while(oled.images[c].isUsed)                    // check which rectangle is unused
    {
        c++;
        if(c >= OLED_MAX_IMAGES_COUNT)
        {
            id = NULL;
            return;
        }
    }

    uint8_t temp;
    do
    {

        i++;
        temp = 1;
        for(uint8_t j = 0; j < OLED_MAX_IMAGES_COUNT; j++)                          // loop in all rectangels to get unique id
        {
            if(oled.images[j].id == i)
            {
                temp = 0;
                break;
            }
        }
    } while(temp != 1);                                 // if any of rectangle have got id == i then increment i and iterate again
    *id = i;
    oled.images[c].id = i;
    oled.images[c].isUsed = 1;
    oled.images[c].x0 = x0;
    oled.images[c].y0 = y0;
    oled.images[c].imageArray = imageArray;
}

void OLED_imageMove(uint8_t id, uint8_t x0, uint8_t y0)
{
    for(uint8_t i = 0; i < OLED_MAX_IMAGES_COUNT; i++)
    {
        if(oled.images[i].id == id)
        {
            oled.images[i].x0 = x0;
            oled.images[i].y0 = y0;
            return;
        }
    }
}
