#include "st7735.h"
#include <stdarg.h>
#include <stdio.h>
void st7735_delay(uint32_t i) {
    volatile uint32_t j;
    for (j=0; j!= i * 100; j++) ;
}

void st7735_sendByte(uint8_t data){
    SPI_I2S_SendData(ST7735_SPI, data);
    while(SPI_I2S_GetFlagStatus(ST7735_SPI, SPI_I2S_FLAG_BSY) == SET);
}

void st7735_sendCommand(uint8_t data){
    GPIO_ResetBits(GPIOA, ST7735_PIN_A0);
    st7735_sendByte(data);
}

void st7735_sendData(uint8_t data){
    GPIO_SetBits(GPIOA, ST7735_PIN_A0);
    st7735_sendByte(data);
}

void st7735_sendColor(Color c){
    st7735_sendData(c.r);
    st7735_sendData(c.g);
    st7735_sendData(c.b);
}

void st7735_setMemoryWindow(uint8_t x0, uint8_t y0, uint8_t x1, uint8_t y1)
{
    st7735_sendCommand(ST7735_CASET);
    st7735_sendData(0x00);
    st7735_sendData(x0);
    st7735_sendData(0x00);
    st7735_sendData(x1);

    st7735_sendCommand(ST7735_RASET);
    st7735_sendData(0x00);
    st7735_sendData(y0);
    st7735_sendData(0x00);
    st7735_sendData(y1);
}

void st7735_drawPixel(uint8_t x, uint8_t y, Color c)
{
    st7735_setMemoryWindow(x,y,x+1,y+1);
    st7735_sendCommand(ST7735_RAMWR);
    st7735_sendColor(c);
}

void st7735_drawRectangle(uint8_t x, uint8_t y, uint8_t width, uint8_t height, Color c)
{
    uint16_t i, length;
    st7735_setMemoryWindow(x,y,x+width-1,y+height-1);
    st7735_sendCommand(ST7735_RAMWR);
    length = width*height;
    for(i=0; i<length; i++){
        st7735_sendColor(c);
    }
}

void st7735_clearScreen(Color color){
    st7735_drawRectangle(0, 0, ST7735_WIDTH, ST7735_HEIGHT, color);
}

char st7735_convertChar(char c){
    if((c >= ' ') && (c<='z')){
        return c-32;
    }
    return 0;
}

uint8_t st7735_printChar(uint8_t x, uint8_t y, Color fontColor, Color backgroundColor, char value){ //!TODO Add limitations for values
    uint8_t j, i;
    if(value == '\n')
        return 0;
    st7735_setMemoryWindow(x, y,x+ ST7735_FONT_WIDTH-1,y+ ST7735_FONT_HEIGHT-1);
    st7735_sendCommand(ST7735_RAMWR);
    uint8_t row = 1;
    char converted = st7735_convertChar(value);
    for( j = 0 ; j < ST7735_FONT_HEIGHT ; j++ ){
        for( i = 0 ; i < ST7735_FONT_WIDTH ; i++ ){
            if( ST7735_FONT_CHARS[converted*5+i] & row ){
                st7735_sendColor(fontColor);
            }else{
                st7735_sendColor(backgroundColor);
            }
        }
        row <<= 1;
    }
    return 1;
}

uint8_t st7735_printString(uint8_t x, uint8_t y, Color fontColor, Color backgroundColor, Orientation orientation,  char *str){
    uint8_t i, length, cursorX, cursorY;
    length = 0;
    cursorX = x;
    cursorY = y;
    for( i = 0 ; (i < ST7735_STRING_MAX_LENGTH) && (str[i] != 0x00); i++ ){
        if(str[i]=='\n'){
            cursorX = x;
            cursorY += ST7735_FONT_HEIGHT;
        }else{
            length += st7735_printChar(cursorX, cursorY, fontColor, backgroundColor, str[i]);
            if(orientation == ST7735_FONT_ORIENTATION_HORIZONTAL)
                cursorX += ST7735_FONT_WIDTH;
            else
                cursorY += ST7735_FONT_HEIGHT;
        }
    }
    return length;
}

uint8_t st7735_printFormattedString(uint8_t x, uint8_t y, Color fontColor, Color backgroundColor, Orientation orientation, char *str, ...){
    uint8_t length;
    va_list args;
    va_start(args, str);
    char buf[ST7735_STRING_MAX_LENGTH];
    vsprintf(buf, str, args);
    length = st7735_printString(x, y, fontColor, backgroundColor, orientation,  buf);
    va_end(args);
    return length;
}

uint8_t st7735_calculateStringWidth(Orientation orientation, uint8_t numberOfChars){//!TODO check for multiline strings
    if(orientation == ST7735_FONT_ORIENTATION_HORIZONTAL)
        return numberOfChars * ST7735_FONT_WIDTH;
    else
        return ST7735_FONT_WIDTH;
}
uint8_t st7735_calculateStringHeight(Orientation orientation, uint8_t numberOfChars){
   if(orientation == ST7735_FONT_ORIENTATION_HORIZONTAL)
        return ST7735_HEIGHT;
    else
        return numberOfChars * ST7735_HEIGHT;
}
uint8_t st7735_calculateMultilineStringWidth(Orientation orientation, char *str){
    uint8_t width, i, x;
    width = 0;
    x = 0;
    if(orientation == ST7735_FONT_ORIENTATION_HORIZONTAL){
        for( i = 0 ; (i < ST7735_STRING_MAX_LENGTH) && (str[i] != 0x00); i++ ){
            if( str[i] == '\n' ){
                x = 0;
            }else{
                x += ST7735_FONT_WIDTH;
                if ( x > width )
                    width = x;
            }
        }
    }else{
        if (str[0] != 0x00)
            width = ST7735_FONT_WIDTH;
    }

    return width;
}
uint8_t st7735_calculateMultilineStringHeight(Orientation orientation, char *str){
    uint8_t height, i;
    height = 0;
    if(orientation == ST7735_FONT_ORIENTATION_HORIZONTAL){
        height = ST7735_FONT_HEIGHT;
        for( i = 0 ; (i < ST7735_STRING_MAX_LENGTH) && (str[i] != 0x00); i++ ){
            if( str[i] == '\n' )
                height += ST7735_FONT_HEIGHT;
        }
    }else{
         for( i = 0 ; (i < ST7735_STRING_MAX_LENGTH) && (str[i] != 0x00); i++ ){
            height += ST7735_FONT_HEIGHT;
        }
    }

    return height;
}
uint8_t st7735_calculateMultilineFormattedStringWidth(Orientation orientation, char *str, ...){
    uint8_t width;
    va_list args;
    va_start(args, str);
    char buf[ST7735_STRING_MAX_LENGTH];
    vsprintf(buf, str, args);
    width = st7735_calculateMultilineStringWidth(orientation, buf);
    va_end(args);
    return width;
}
uint8_t st7735_calculateMultilineFormattedStringHeight(Orientation orientation, char *str, ...){
    uint8_t height;
    va_list args;
    va_start(args, str);
    char buf[ST7735_STRING_MAX_LENGTH];
    vsprintf(buf, str, args);
    height = st7735_calculateMultilineStringHeight(orientation, buf);
    va_end(args);
    return height;
}
void st7735_initGPIO(void){
    GPIO_InitTypeDef GPIO_InitStructure;
    SPI_InitTypeDef SPI_InitStructure;

    RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOA, ENABLE);
    RCC_APB2PeriphClockCmd(RCC_APB2Periph_SPI1, ENABLE);
    RCC_APB2PeriphClockCmd(RCC_APB2Periph_AFIO, ENABLE);

    GPIO_InitStructure.GPIO_Pin = ST7735_PIN_A0;
    GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
    GPIO_InitStructure.GPIO_Mode = GPIO_Mode_Out_PP;
    GPIO_Init(GPIOA, &GPIO_InitStructure);

    GPIO_InitStructure.GPIO_Pin = ST7735_PIN_SCK | ST7735_PIN_SDA;
    GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
    GPIO_InitStructure.GPIO_Mode = GPIO_Mode_AF_PP;
    GPIO_Init(GPIOA, &GPIO_InitStructure);


    SPI_InitStructure.SPI_Direction = SPI_Direction_2Lines_FullDuplex;
    SPI_InitStructure.SPI_Mode = SPI_Mode_Master;
    SPI_InitStructure.SPI_DataSize = SPI_DataSize_8b;
    SPI_InitStructure.SPI_CPOL = SPI_CPOL_Low;
    SPI_InitStructure.SPI_CPHA = SPI_CPHA_1Edge;
    SPI_InitStructure.SPI_NSS = SPI_NSS_Soft;
    SPI_InitStructure.SPI_BaudRatePrescaler = SPI_BaudRatePrescaler_8;
    SPI_InitStructure.SPI_FirstBit = SPI_FirstBit_MSB;
    SPI_Init(ST7735_SPI, &SPI_InitStructure);
    SPI_Cmd(ST7735_SPI, ENABLE);
    SPI_NSSInternalSoftwareConfig(ST7735_SPI, SPI_NSSInternalSoft_Set);
}

void st7735_initDisplay(){
    st7735_sendCommand(ST7735_SWRESET); //reset
    st7735_delay(500);
    st7735_sendCommand(ST7735_SLPOUT);  //sleep out
    st7735_delay(500);
    st7735_sendCommand(ST7735_DISPON);  //display on
    st7735_delay(500);
}


void st7735_init(void){
    st7735_initGPIO();
    st7735_initDisplay();
}

const uint8_t ST7735_FONT_CHARS[] = {
0x00,0x00,0x00,0x00,0x00,
0x00,0x00,0x9E,0x00,0x00,
0x00,0x0E,0x00,0x0E,0x00,
0x28,0xFE,0x28,0xFE,0x28,
0x48,0x54,0xFE,0x54,0x24,
0x46,0x26,0x10,0xC8,0xC4,
0x6C,0x92,0xAA,0x44,0x80,
0x00,0x0A,0x06,0x00,0x00,
0x00,0x38,0x44,0x82,0x00,
0x00,0x82,0x44,0x38,0x00,
0x28,0x10,0x7C,0x10,0x28,
0x10,0x10,0x7C,0x10,0x10,
0x00,0xA0,0x60,0x00,0x00,
0x10,0x10,0x10,0x10,0x10,
0x00,0xC0,0xC0,0x00,0x00,
0x40,0x20,0x10,0x08,0x04,
0x7C,0xA2,0x92,0x8A,0x7C,
0x00,0x84,0xFE,0x80,0x00,
0x84,0xC2,0xA2,0x92,0x8C,
0x42,0x82,0x8A,0x96,0x62,
0x30,0x28,0x24,0xFE,0x20,
0x4E,0x8A,0x8A,0x8A,0x72,
0x78,0x94,0x92,0x92,0x60,
0x02,0xE2,0x12,0x0A,0x06,
0x6C,0x92,0x92,0x92,0x6C,
0x0C,0x92,0x92,0x52,0x3C,
0x00,0x6C,0x6C,0x00,0x00,
0x00,0xAC,0x6C,0x00,0x00,
0x10,0x28,0x44,0x82,0x00,
0x28,0x28,0x28,0x28,0x28,
0x82,0x44,0x28,0x10,0x00,
0x04,0x02,0xA2,0x12,0x0C,
0x64,0x92,0xF2,0x82,0x7C,
0xFC,0x22,0x22,0x22,0xFC,
0xFE,0x92,0x92,0x92,0x6C,
0x7C,0x82,0x82,0x82,0x44,
0xFE,0x82,0x82,0x82,0x7C,
0xFE,0x92,0x92,0x92,0x82,
0xFE,0x12,0x12,0x12,0x02,
0x7C,0x82,0x92,0x92,0xF4,
0xFE,0x10,0x10,0x10,0xFE,
0x00,0x82,0xFE,0x82,0x00,
0x40,0x80,0x82,0x7E,0x02,
0xFE,0x10,0x28,0x44,0x82,
0xFE,0x80,0x80,0x80,0x80,
0xFE,0x04,0x08,0x04,0xFE,
0xFE,0x08,0x10,0x20,0xFE,
0x7C,0x82,0x82,0x82,0x7C,
0xFE,0x12,0x12,0x12,0x0C,
0x7C,0x82,0xA2,0x42,0xBC,
0xFE,0x12,0x32,0x52,0x8C,
0x8C,0x92,0x92,0x92,0x62,
0x02,0x02,0xFE,0x02,0x02,
0x7E,0x80,0x80,0x80,0x7E,
0x3E,0x40,0x80,0x40,0x3E,
0x7E,0x80,0x70,0x80,0x7E,
0xC6,0x28,0x10,0x28,0xC6,
0x0E,0x10,0xE0,0x10,0x0E,
0xC2,0xA2,0x92,0x8A,0x86,
0x00,0xFE,0x82,0x82,0x00,
0x0E,0x10,0xE0,0x10,0x0E,
0x00,0x82,0x82,0xFE,0x00,
0x08,0x04,0x02,0x04,0x08,
0x80,0x80,0x80,0x80,0x80,
0x00,0x02,0x04,0x08,0x00,
0x40,0xA8,0xA8,0xA8,0xF0,
0xFE,0x90,0x88,0x88,0x70,
0x70,0x88,0x88,0x88,0x40,
0x70,0x88,0x88,0x90,0xFE,
0x70,0xA8,0xA8,0xA8,0x30,
0x10,0xFC,0x12,0x02,0x04,
0x0C,0x92,0x92,0x92,0x7E,
0xFE,0x10,0x08,0x08,0xF0,
0x00,0x88,0xFA,0x80,0x00,
0x40,0x80,0x88,0x7A,0x00,
0xFE,0x20,0x50,0x88,0x00,
0x00,0x82,0xFE,0x80,0x00,
0xF8,0x08,0xF0,0x08,0xF0,
0xF8,0x10,0x08,0x08,0xF0,
0x70,0x88,0x88,0x88,0x70,
0xF8,0x28,0x28,0x28,0x10,
0x10,0x28,0x28,0x30,0xF8,
0xF8,0x10,0x08,0x08,0x10,
0x90,0xA8,0xA8,0xA8,0x48,
0x04,0x7E,0x84,0x80,0x40,
0x78,0x80,0x80,0x40,0xF8,
0x38,0x40,0x80,0x40,0x38,
0x78,0x80,0x60,0x80,0x78,
0x88,0x50,0x20,0x50,0x88,
0x18,0xA0,0xA0,0xA0,0x78,
0x88,0xC8,0xA8,0x98,0x88,
};
