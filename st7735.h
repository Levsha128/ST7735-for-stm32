#include "stm32f10x.h"
#include "stm32f10x_spi.h"
#include "stm32f10x_gpio.h"

#define ST7735_WIDTH                        (128)
#define ST7735_HEIGHT                       (160)
#define ST7735_FONT_WIDTH                   (5)
#define ST7735_FONT_HEIGHT                  (8)
#define ST7735_FONT_ORIENTATION_VERTICAL    (1)
#define ST7735_FONT_ORIENTATION_HORIZONTAL  (0)
#define ST7735_FONT_ORIENTATION_DEFAULT ST7735_ORIENTATION_HORIZONTAL
#define ST7735_STRING_MAX_LENGTH            (50)

#define ST7735_SWRESET  (0x01)
#define ST7735_SLPOUT   (0x11)
#define ST7735_DISPON   (0x29)
#define ST7735_CASET    (0x2A)
#define ST7735_RASET    (0x2B)
#define ST7735_RAMWR    (0x2C)


#define ST7735_PIN_A0 (GPIO_Pin_3)
#define ST7735_PIN_SDA (GPIO_Pin_5)
#define ST7735_PIN_SCK (GPIO_Pin_7)
#define ST7735_SPI (SPI1)



typedef struct {
    uint8_t r, g, b;
} Color;

typedef uint8_t Orientation;

extern const uint8_t ST7735_FONT_CHARS[];

void st7735_delay(uint32_t i);
void st7735_sendByte(uint8_t data);
void st7735_sendCommand(uint8_t data);
void st7735_sendData(uint8_t data);
void st7735_setMemoryWindow(uint8_t x0, uint8_t y0, uint8_t x1, uint8_t y1);
void st7735_drawPixel(uint8_t x, uint8_t y, Color color);
void st7735_drawRectangle(uint8_t x, uint8_t y, uint8_t width, uint8_t height, Color color);
void st7735_initGPIO(void);
void st7735_initDisplay(void);
void st7735_init(void);
void st7735_clearScreen(Color color);
uint8_t st7735_printChar(uint8_t x, uint8_t y, Color fontColor, Color backgroundColor, char value);
uint8_t st7735_printString(uint8_t x, uint8_t y, Color fontColor, Color backgroundColor, Orientation orientation,  char *value);
uint8_t st7735_printFormattedString(uint8_t x, uint8_t y, Color fontColor, Color backgroundColor, Orientation orientation, char *value, ...);
uint8_t st7735_calculateStringWidth(Orientation orientation, uint8_t numberOfChars);
uint8_t st7735_calculateStringHeight(Orientation orientation, uint8_t numberOfChars);
uint8_t st7735_calculateMultilineStringWidth(Orientation orientation, char *str);
uint8_t st7735_calculateMultilineStringHeight(Orientation orientation, char *str);
uint8_t st7735_calculateMultilineFormattedStringWidth(Orientation orientation, char *str, ...);
uint8_t st7735_calculateMultilineFormattedStringHeight(Orientation orientation, char *str, ...);


#define ST7735_COLOR_ALICE_BLUE 				{0xF0, 0xF8, 0xFF}
#define ST7735_COLOR_ANTIQUE_WHITE 				{0xFA, 0xEB, 0xD7}
#define ST7735_COLOR_AQUA 						{0x00, 0xFF, 0xFF}
#define ST7735_COLOR_AQUAMARINE 				{0x7F, 0xFF, 0xD4}
#define ST7735_COLOR_AZURE 						{0xF0, 0xFF, 0xFF}
#define ST7735_COLOR_BEIGE 						{0xF5, 0xF5, 0xDC}
#define ST7735_COLOR_BISQUE 					{0xFF, 0xE4, 0xC4}
#define ST7735_COLOR_BLACK 						{0x00, 0x00, 0x00}
#define ST7735_COLOR_BLANCHED_ALMOND 			{0xFF, 0xEB, 0xCD}
#define ST7735_COLOR_BLUE 						{0x00, 0x00, 0xFF}
#define ST7735_COLOR_BLUE_VIOLET 				{0x8A, 0x2B, 0xE2}
#define ST7735_COLOR_BROWN 						{0xA5, 0x2A, 0x2A}
#define ST7735_COLOR_BURLY_WOOD 				{0xDE, 0xB8, 0x87}
#define ST7735_COLOR_CADET_BLUE 				{0x5F, 0x9E, 0xA0}
#define ST7735_COLOR_CHARTREUSE 				{0x7F, 0xFF, 0x00}
#define ST7735_COLOR_CHOCOLATE 					{0xD2, 0x69, 0x1E}
#define ST7735_COLOR_CORAL 						{0xFF, 0x7F, 0x50}
#define ST7735_COLOR_CORNFLOWER_BLUE 			{0x64, 0x95, 0xED}
#define ST7735_COLOR_CORNSILK 					{0xFF, 0xF8, 0xDC}
#define ST7735_COLOR_CRIMSON 					{0xDC, 0x14, 0x3C}
#define ST7735_COLOR_CYAN 						{0x00, 0xFF, 0xFF}
#define ST7735_COLOR_DARK_BLUE 					{0x00, 0x00, 0x8B}
#define ST7735_COLOR_DARK_CYAN 					{0x00, 0x8B, 0x8B}
#define ST7735_COLOR_DARK_GOLDEN_ROD 			{0xB8, 0x86, 0x0B}
#define ST7735_COLOR_DARK_GRAY 					{0xA9, 0xA9, 0xA9}
#define ST7735_COLOR_DARK_GREEN 				{0x00, 0x64, 0x00}
#define ST7735_COLOR_DARK_KHAKI 				{0xBD, 0xB7, 0x6B}
#define ST7735_COLOR_DARK_MAGENTA 				{0x8B, 0x00, 0x8B}
#define ST7735_COLOR_DARK_OLIVE_GREEN 			{0x55, 0x6B, 0x2F}
#define ST7735_COLOR_DARK_ORANGE 				{0xFF, 0x8C, 0x00}
#define ST7735_COLOR_DARK_ORCHID 				{0x99, 0x32, 0xCC}
#define ST7735_COLOR_DARK_RED 					{0x8B, 0x00, 0x00}
#define ST7735_COLOR_DARK_SALMON 				{0xE9, 0x96, 0x7A}
#define ST7735_COLOR_DARK_SEA_GREEN 			{0x8F, 0xBC, 0x8F}
#define ST7735_COLOR_DARK_SLATE_BLUE 			{0x48, 0x3D, 0x8B}
#define ST7735_COLOR_DARK_SLATE_GRAY 			{0x2F, 0x4F, 0x4F}
#define ST7735_COLOR_DARK_TURQUOISE 			{0x00, 0xCE, 0xD1}
#define ST7735_COLOR_DARK_VIOLET 				{0x94, 0x00, 0xD3}
#define ST7735_COLOR_DEEP_PINK 					{0xFF, 0x14, 0x93}
#define ST7735_COLOR_DEEP_SKY_BLUE 				{0x00, 0xBF, 0xFF}
#define ST7735_COLOR_DIM_GRAY 					{0x69, 0x69, 0x69}
#define ST7735_COLOR_DODGER_BLUE 				{0x1E, 0x90, 0xFF}
#define ST7735_COLOR_FIRE_BRICK 				{0xB2, 0x22, 0x22}
#define ST7735_COLOR_FLORAL_WHITE 				{0xFF, 0xFA, 0xF0}
#define ST7735_COLOR_FOREST_GREEN 				{0x22, 0x8B, 0x22}
#define ST7735_COLOR_FUCHSIA 					{0xFF, 0x00, 0xFF}
#define ST7735_COLOR_GAINSBORO 					{0xDC, 0xDC, 0xDC}
#define ST7735_COLOR_GHOST_WHITE 				{0xF8, 0xF8, 0xFF}
#define ST7735_COLOR_GOLD 						{0xFF, 0xD7, 0x00}
#define ST7735_COLOR_GOLDEN_ROD 				{0xDA, 0xA5, 0x20}
#define ST7735_COLOR_GRAY 						{0x80, 0x80, 0x80}
#define ST7735_COLOR_GREEN 						{0x00, 0x80, 0x00}
#define ST7735_COLOR_GREEN_YELLOW 				{0xAD, 0xFF, 0x2F}
#define ST7735_COLOR_HONEY_DEW 					{0xF0, 0xFF, 0xF0}
#define ST7735_COLOR_HOT_PINK 					{0xFF, 0x69, 0xB4}
#define ST7735_COLOR_INDIAN_RED  				{0xCD, 0x5C, 0x5C}
#define ST7735_COLOR_INDIGO  					{0x4B, 0x00, 0x82}
#define ST7735_COLOR_IVORY 						{0xFF, 0xFF, 0xF0}
#define ST7735_COLOR_KHAKI 						{0xF0, 0xE6, 0x8C}
#define ST7735_COLOR_LAVENDER 					{0xE6, 0xE6, 0xFA}
#define ST7735_COLOR_LAVENDER_BLUSH 			{0xFF, 0xF0, 0xF5}
#define ST7735_COLOR_LAWN_GREEN 				{0x7C, 0xFC, 0x00}
#define ST7735_COLOR_LEMON_CHIFFON 				{0xFF, 0xFA, 0xCD}
#define ST7735_COLOR_LIGHT_BLUE 				{0xAD, 0xD8, 0xE6}
#define ST7735_COLOR_LIGHT_CORAL 				{0xF0, 0x80, 0x80}
#define ST7735_COLOR_LIGHT_CYAN 				{0xE0, 0xFF, 0xFF}
#define ST7735_COLOR_LIGHT_GOLDEN_ROD_YELLOW 	{0xFA, 0xFA, 0xD2}
#define ST7735_COLOR_LIGHT_GRAY 				{0xD3, 0xD3, 0xD3}
#define ST7735_COLOR_LIGHT_GREEN 				{0x90, 0xEE, 0x90}
#define ST7735_COLOR_LIGHT_PINK 				{0xFF, 0xB6, 0xC1}
#define ST7735_COLOR_LIGHT_SALMON 				{0xFF, 0xA0, 0x7A}
#define ST7735_COLOR_LIGHT_SEA_GREEN 			{0x20, 0xB2, 0xAA}
#define ST7735_COLOR_LIGHT_SKY_BLUE 			{0x87, 0xCE, 0xFA}
#define ST7735_COLOR_LIGHT_SLATE_GRAY 			{0x77, 0x88, 0x99}
#define ST7735_COLOR_LIGHT_STEEL_BLUE 			{0xB0, 0xC4, 0xDE}
#define ST7735_COLOR_LIGHT_YELLOW 				{0xFF, 0xFF, 0xE0}
#define ST7735_COLOR_LIME 						{0x00, 0xFF, 0x00}
#define ST7735_COLOR_LIME_GREEN 				{0x32, 0xCD, 0x32}
#define ST7735_COLOR_LINEN 						{0xFA, 0xF0, 0xE6}
#define ST7735_COLOR_MAGENTA 					{0xFF, 0x00, 0xFF}
#define ST7735_COLOR_MAROON 					{0x80, 0x00, 0x00}
#define ST7735_COLOR_MEDIUM_AQUA_MARINE 		{0x66, 0xCD, 0xAA}
#define ST7735_COLOR_MEDIUM_BLUE 				{0x00, 0x00, 0xCD}
#define ST7735_COLOR_MEDIUM_ORCHID 				{0xBA, 0x55, 0xD3}
#define ST7735_COLOR_MEDIUM_PURPLE 				{0x93, 0x70, 0xDB}
#define ST7735_COLOR_MEDIUM_SEA_GREEN 			{0x3C, 0xB3, 0x71}
#define ST7735_COLOR_MEDIUM_SLATE_BLUE 			{0x7B, 0x68, 0xEE}
#define ST7735_COLOR_MEDIUM_SPRING_GREEN 		{0x00, 0xFA, 0x9A}
#define ST7735_COLOR_MEDIUM_TURQUOISE 			{0x48, 0xD1, 0xCC}
#define ST7735_COLOR_MEDIUM_VIOLET_RED 			{0xC7, 0x15, 0x85}
#define ST7735_COLOR_MIDNIGHT_BLUE 				{0x19, 0x19, 0x70}
#define ST7735_COLOR_MINT_CREAM 				{0xF5, 0xFF, 0xFA}
#define ST7735_COLOR_MISTY_ROSE 				{0xFF, 0xE4, 0xE1}
#define ST7735_COLOR_MOCCASIN 					{0xFF, 0xE4, 0xB5}
#define ST7735_COLOR_NAVAJO_WHITE 				{0xFF, 0xDE, 0xAD}
#define ST7735_COLOR_NAVY 						{0x00, 0x00, 0x80}
#define ST7735_COLOR_OLD_LACE 					{0xFD, 0xF5, 0xE6}
#define ST7735_COLOR_OLIVE 						{0x80, 0x80, 0x00}
#define ST7735_COLOR_OLIVE_DRAB 				{0x6B, 0x8E, 0x23}
#define ST7735_COLOR_ORANGE 					{0xFF, 0xA5, 0x00}
#define ST7735_COLOR_ORANGE_RED 				{0xFF, 0x45, 0x00}
#define ST7735_COLOR_ORCHID 					{0xDA, 0x70, 0xD6}
#define ST7735_COLOR_PALE_GOLDEN_ROD 			{0xEE, 0xE8, 0xAA}
#define ST7735_COLOR_PALE_GREEN 				{0x98, 0xFB, 0x98}
#define ST7735_COLOR_PALE_TURQUOISE 			{0xAF, 0xEE, 0xEE}
#define ST7735_COLOR_PALE_VIOLET_RED 			{0xDB, 0x70, 0x93}
#define ST7735_COLOR_PAPAYA_WHIP 				{0xFF, 0xEF, 0xD5}
#define ST7735_COLOR_PEACH_PUFF 				{0xFF, 0xDA, 0xB9}
#define ST7735_COLOR_PERU 						{0xCD, 0x85, 0x3F}
#define ST7735_COLOR_PINK 						{0xFF, 0xC0, 0xCB}
#define ST7735_COLOR_PLUM 						{0xDD, 0xA0, 0xDD}
#define ST7735_COLOR_POWDER_BLUE 				{0xB0, 0xE0, 0xE6}
#define ST7735_COLOR_PURPLE 					{0x80, 0x00, 0x80}
#define ST7735_COLOR_RED 						{0xFF, 0x00, 0x00}
#define ST7735_COLOR_ROSY_BROWN 				{0xBC, 0x8F, 0x8F}
#define ST7735_COLOR_ROYAL_BLUE 				{0x41, 0x69, 0xE1}
#define ST7735_COLOR_SADDLE_BROWN 				{0x8B, 0x45, 0x13}
#define ST7735_COLOR_SALMON 					{0xFA, 0x80, 0x72}
#define ST7735_COLOR_SANDY_BROWN 				{0xF4, 0xA4, 0x60}
#define ST7735_COLOR_SEA_GREEN 					{0x2E, 0x8B, 0x57}
#define ST7735_COLOR_SEA_SHELL 					{0xFF, 0xF5, 0xEE}
#define ST7735_COLOR_SIENNA 					{0xA0, 0x52, 0x2D}
#define ST7735_COLOR_SILVER 					{0xC0, 0xC0, 0xC0}
#define ST7735_COLOR_SKY_BLUE 					{0x87, 0xCE, 0xEB}
#define ST7735_COLOR_SLATE_BLUE 				{0x6A, 0x5A, 0xCD}
#define ST7735_COLOR_SLATE_GRAY 				{0x70, 0x80, 0x90}
#define ST7735_COLOR_SNOW 						{0xFF, 0xFA, 0xFA}
#define ST7735_COLOR_SPRING_GREEN 				{0x00, 0xFF, 0x7F}
#define ST7735_COLOR_STEEL_BLUE 				{0x46, 0x82, 0xB4}
#define ST7735_COLOR_TAN 						{0xD2, 0xB4, 0x8C}
#define ST7735_COLOR_TEAL 						{0x00, 0x80, 0x80}
#define ST7735_COLOR_THISTLE 					{0xD8, 0xBF, 0xD8}
#define ST7735_COLOR_TOMATO 					{0xFF, 0x63, 0x47}
#define ST7735_COLOR_TURQUOISE 					{0x40, 0xE0, 0xD0}
#define ST7735_COLOR_VIOLET 					{0xEE, 0x82, 0xEE}
#define ST7735_COLOR_WHEAT 						{0xF5, 0xDE, 0xB3}
#define ST7735_COLOR_WHITE 						{0xFF, 0xFF, 0xFF}
#define ST7735_COLOR_WHITE_SMOKE 				{0xF5, 0xF5, 0xF5}
#define ST7735_COLOR_YELLOW 					{0xFF, 0xFF, 0x00}
#define ST7735_COLOR_YELLOW_GREEN 				{0x9A, 0xCD, 0x32}
