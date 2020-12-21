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

typedef enum
{
    TEXT_FIELD = 0,
    LINE,
    RECTANGLE,
    IMAGE
} drawable_e;

// common part of all drawable objects
typedef struct
{
    drawable_e type;
    uint8_t x0;
    uint8_t y0;
    uint8_t isUsed : 1;
} drawable_base_t;

// === TEXT FIELD ===
typedef struct
{
    char *              text;                                           // pointer to character sequence
                                                                        // If it is different from zero it will be
                                                                        // updated in buffer and decremented so both firstBuffer
                                                                        // and secondBuffer becomes updated.
} textField_t;

// === LINE ===
typedef struct
{
    uint8_t             x1;                                              // end point of line x coordinate
    uint8_t             y1;                                              // end point of line y coordinate
} line_t;

// === RECTANGLE ===
typedef struct
{
    uint8_t             width;                                           // rectangle width
    uint8_t             height;                                          // rectangle height
} rectangle_t;

// === IMAGE ===
typedef struct
{
    uint8_t * imageArray;                                               // pointer to an array with image representation
} image_t;

union drawable_specific
{
    textField_t                     textField;
    line_t                          line;
    rectangle_t                     rectangle;
    image_t                         image;
};

typedef struct
{
    drawable_base_t common;
    union drawable_specific spec;
}drawable_t;

typedef struct
{
    uint8_t             firstBuffer[OLED_NUM_OF_PAGES*OLED_X_SIZE];      // two buffers used alternately for updating display
    uint8_t             secondBufffer[OLED_NUM_OF_PAGES*OLED_X_SIZE];
    uint8_t             addressArray[3];                                 // temp array used to send GRAM address through I2C
    uint8_t *           currentBuffer;                                   // pointer to currently used buffer
    drawable_t          drawables[OLED_MAX_DRAWABLES_COUNT];             // drrawable objects
    uint8_t             firstBufferAvailable : 1;
} oled_t;

oled_t oled;


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

#define SET_PIXEL(x, y)(*oled.currentBuffer + x + OLED_X_SIZE*((uint8_t)(y / 8) )) |= 0x01 << y % 8;

static void setPixel(uint8_t x, uint8_t y)
{
    *(oled.currentBuffer + x + OLED_X_SIZE*( (uint8_t) (y / 8) )) |= 0x01 << y % 8;
}

void clearScreen()
{
    for(uint8_t v = 0; v < OLED_NUM_OF_PAGES; v++){
        for(uint8_t c = 0; c < OLED_X_SIZE; c++){
            *(oled.currentBuffer + v*OLED_X_SIZE + c) = 0;
        }
    }
}

static void printText(uint8_t x0, uint8_t y0, char * text)
{

    if(x0 >= OLED_X_SIZE || (y0 + 8)>= OLED_Y_SIZE)
        return;

    uint8_t i = 0;
    uint8_t v = y0 / 8;
    uint8_t rem = y0 % 8;

    while(text[i] != '\0')
    {
        if(rem)
        {
            for(uint8_t j = 0; j < 5; j++)
            {
                *(oled.currentBuffer + v*OLED_X_SIZE + x0) |= (font_ASCII[text[i] - ' '][j] << rem);
                *(oled.currentBuffer + (v+1)*OLED_X_SIZE + x0++) |= (font_ASCII[text[i] - ' '][j] >> (8 - rem));
            }
            x0++;
        }
        else
        {
            for(uint8_t j = 0; j < 5; j++)
                *(oled.currentBuffer + v*OLED_X_SIZE + x0++) = font_ASCII[text[i] - ' '][j];
            x0++;
        }
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
            *(oled.currentBuffer + (( uint8_t ) y / 8)*OLED_X_SIZE + ( uint8_t ) x ) |= 1 << ( (uint8_t) y % 8);
            yTemp += tan;
            if(yTemp >= 1)
            {
                if(yDir == 1)
                    y++;
                else
                    y--;
                yTemp = yTemp - (int)yTemp;
            }

            xTemp += oneOverTan;
            if(xTemp >= 1)
            {
                if(xDir == 1)
                    x++;
                else
                    x--;
                xTemp = xTemp - (int)xTemp;
            }
        }
    }
}

static void drawRect(uint8_t x0, uint8_t y0, uint8_t x1, uint8_t y1, enum OLED_Color color){
    uint8_t rem0 = y0 % 8;  // 6
    uint8_t rem1 = y1 % 8;  // 1

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
                *(oled.currentBuffer + v*OLED_X_SIZE + c++) |= 0xFF >> ( 8 - rem1 );
            else
                *(oled.currentBuffer + v*OLED_X_SIZE + c++) &= ~(0xFF >> ( 8 - rem1 ));
        }
    }
}

static void drawImage(uint8_t x0, uint8_t y0,const uint8_t image[])
{
    uint8_t width = image[0], height = image[1];
    uint8_t x1 = x0 + width - 1;
    uint8_t y1 = y0 + height - 1;
    uint8_t rem0 = y0 % 8;  // 6

    uint8_t v = y0 / 8;
    uint8_t c;

    uint16_t i = 2;

    if(rem0 == 0){
        while(v <= y1 / 8)
        {
            c = x0;
            while(c <= x1)
            {
                *(oled.currentBuffer + v*OLED_X_SIZE + c++) |= image[i++];
            }
            v++;
        }
    }
    else
    {
        c = x0;
        while(c <= x1)
        {
            *(oled.currentBuffer + v*OLED_X_SIZE + c++) |= image[i++];
        }

        v++;
        while(v <= y1 / 8)
        {
            c = x0;
            while(c <= x1)
            {
                *(oled.currentBuffer + v*OLED_X_SIZE + c) |= image[i - width] >> (8 - rem0);
                if(v != y1 / 8)
                    *(oled.currentBuffer + v*OLED_X_SIZE + c) |= image[i] << rem0;
                i++;
                c++;
            }
            v++;
        }
    }
}

static void getNextFreeId(uint8_t * id)
{
    *id = 0;
    while(oled.drawables[*id].common.isUsed)
    {
        (*id)++;
        if(*id == OLED_MAX_DRAWABLES_COUNT)   // all drawable slots already used
        {
            id = NULL;
            return;
        }
    }
    return;
}
//---------------------------------------------------------------------------------------
/* API functions */

// OK
void OLED_Init()
{
    uint8_t i = 0;
    oled.currentBuffer = oled.firstBuffer;
    for(i = 0; i < OLED_MAX_DRAWABLES_COUNT; i++)
    {
        oled.drawables[i].common.isUsed = 0;
    }

    sendCommandStream(initSequence, 32);
    clearScreen();
    OLED_update();
}

void OLED_update()
{
    clearScreen();

    // updata drawable objects on buffer
    for(uint8_t i = 0; i < OLED_MAX_DRAWABLES_COUNT; i++)
    {
        if(oled.drawables[i].common.isUsed)
        {
            switch(oled.drawables[i].common.type)
            {
            case TEXT_FIELD:
                printText(oled.drawables[i].common.x0, oled.drawables[i].common.y0,
                          oled.drawables[i].spec.textField.text);
                break;
            case LINE:
                drawLine(oled.drawables[i].common.x0, oled.drawables[i].common.y0,
                         oled.drawables[i].spec.line.x1, oled.drawables[i].spec.line.y1);
                break;
            case RECTANGLE:
                drawRect(oled.drawables[i].common.x0, oled.drawables[i].common.y0,
                        oled.drawables[i].common.x0 + oled.drawables[i].spec.rectangle.width,
                         oled.drawables[i].common.y0 + oled.drawables[i].spec.rectangle.height,
                         WHITE);
                break;
            case IMAGE:
                drawImage(oled.drawables[i].common.x0, oled.drawables[i].common.y0,
                          oled.drawables[i].spec.image.imageArray);
                break;
            }
        }
    }

    // send buffer to OLED
    for(uint8_t v = 0; v < OLED_NUM_OF_PAGES; v++){
        setAddress(v, 0);
        HAL_I2C_Mem_Write(&OLED_I2C_HANDLE, OLED_ADDRESS, OLED_CONTROL_BYTE_ | _OLED_DATA | _OLED_MULTIPLE_BYTES,
                1, (uint8_t * )(oled.currentBuffer + v*OLED_X_SIZE), OLED_X_SIZE, HAL_MAX_DELAY);
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

void OLED_moveObject(uint8_t id, uint8_t x0, uint8_t y0)
{
    oled.drawables[id].common.x0 = x0;
    oled.drawables[id].common.y0 = y0;
}

void OLED_deleteObject(uint8_t id)
{
    oled.drawables[id].common.isUsed = 0;
}


// === TEXT FIELD ===
void OLED_createTextField(uint8_t * id, uint8_t x0, uint8_t y0, char* text)
{
    getNextFreeId(id);
    if(id == NULL)
        return;                 // all ids used

    oled.drawables[*id].common.isUsed = 1;
    oled.drawables[*id].common.type = TEXT_FIELD;
    oled.drawables[*id].common.x0 = x0;
    oled.drawables[*id].common.y0 = y0;
    oled.drawables[*id].spec.textField.text = text;
}

void OLED_textFieldSetText(uint8_t id, char * text)
{
    oled.drawables[id].spec.textField.text = text;
}

// === LINE ===
void OLED_createLine(uint8_t * id, uint8_t x0, uint8_t y0, uint8_t x1, uint8_t y1)
{
    getNextFreeId(id);
    if(id == NULL)
        return;                 // all ids used

    oled.drawables[*id].common.isUsed = 1;
    oled.drawables[*id].common.type = LINE;
    oled.drawables[*id].common.x0 = x0;
    oled.drawables[*id].common.y0 = y0;
    oled.drawables[*id].spec.line.x1 = x1;
    oled.drawables[*id].spec.line.y1 = y1;
}


void OLED_lineMoveEnd(uint8_t id, uint8_t x1, uint8_t y1)
{
    oled.drawables[id].spec.line.x1 = x1;
    oled.drawables[id].spec.line.y1 = y1;
}

// === RECTANGLE ===
void OLED_createRectangle(uint8_t * id,  uint8_t x0, uint8_t y0, uint8_t width, uint8_t height)
{
    getNextFreeId(id);
    if(id == NULL)
        return;                 // all ids used

    oled.drawables[*id].common.isUsed = 1;
    oled.drawables[*id].common.type = RECTANGLE;
    oled.drawables[*id].common.x0 = x0;
    oled.drawables[*id].common.y0 = y0;
    oled.drawables[*id].spec.rectangle.width = width;
    oled.drawables[*id].spec.rectangle.height = height;
}

void OLED_rectangleSetDimensions(uint8_t id, uint8_t width, uint8_t height)
{
    oled.drawables[id].spec.rectangle.width = width;
    oled.drawables[id].spec.rectangle.height = height;
}

// === IMAGE ===
void OLED_createImage(uint8_t * id, uint8_t x0, uint8_t y0, uint8_t * imageArray)
{
    getNextFreeId(id);
    if(id == NULL)
        return;                 // all ids used

    oled.drawables[*id].common.isUsed = 1;
    oled.drawables[*id].common.type = IMAGE;
    oled.drawables[*id].common.x0 = x0;
    oled.drawables[*id].common.y0 = y0;
    oled.drawables[*id].spec.image.imageArray = imageArray;
}


