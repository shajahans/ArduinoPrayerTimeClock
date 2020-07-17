///////////////////////////////////
//TFT
//---
//CS  - D8 - 15
//SDC - D5 - 14
//SDA - D7 - 13
//DC  - D0 - 16
//RST - Rt - -1
//
//RTC
//---
//DAT - D2 - 4
//CLK - D1 - 5
//RST - Rx - 3
//
//Button
//------
//B1 - D3 - 0
//B2 - D4 - 2
//
//Buzzer - Tx - 1
////////////////////////////////////

#include <virtuabotixRTC.h>   // include Arduino Wire library (required for DS1302 devices)
#include <Time.h>
#include <TimeAlarms.h>
#include <ESP8266WiFi.h>
#include <TFT_22_ILI9225.h>
#include <SPI.h>
 
virtuabotixRTC myRTC(5, 4, -1); //Wiring of the RTC (CLK,DAT,RST)

#define TFT_RST -1
#define TFT_RS  16
#define TFT_CS  15  // SS
#define TFT_SDI 13  // MOSI
#define TFT_CLK 14  // SCK
#define TFT_LED 0   // 0 if wired to +5V directly
#define TFT_BRIGHTNESS 100 // Initial brightness of TFT backlight (optional)

// Use hardware SPI (faster - on Uno: 13-SCK, 12-MISO, 11-MOSI)
TFT_22_ILI9225 tft = TFT_22_ILI9225(TFT_RST, TFT_RS, TFT_CS, TFT_LED, TFT_BRIGHTNESS);
// Use software SPI (slower)
//TFT_22_ILI9225 tft = TFT_22_ILI9225(TFT_RST, TFT_RS, TFT_CS, TFT_SDI, TFT_CLK, TFT_LED, TFT_BRIGHTNESS);

#define MAX_GEN_COUNT 500

#define GRIDX 80
#define GRIDY 64
#define CELLXY 2

//#define WAIT 500

#define GEN_DELAY 10 // Set a delay between each generation to slow things down

//Current grid and newgrid arrays are needed
uint8_t grid[GRIDX][GRIDY];

//The new grid for the next generation
uint8_t newgrid[GRIDX][GRIDY];

//Number of generations
uint16_t genCount = 0;

#define button1    0    // button B1
#define button2    2    // button B2
#define BUZZER 3

void SetBlinkAlarm(int, int, int);

int Ptable [366][12] = {{5,17,12,22,15,39,18,4,19,20,6,41},
{5,18,12,23,15,40,18,5,19,21,6,42},
{5,18,12,23,15,40,18,5,19,21,6,42},
{5,19,12,24,15,41,18,6,19,22,6,43},
{5,19,12,24,15,41,18,6,19,22,6,43},
{5,20,12,25,15,42,18,8,19,23,6,44},
{5,20,12,25,15,42,18,8,19,23,6,44},
{5,20,12,25,15,42,18,8,19,23,6,44},
{5,21,12,26,15,43,18,9,19,24,6,44},
{5,21,12,26,15,43,18,9,19,24,6,44},
{5,21,12,27,15,44,18,10,19,25,6,45},
{5,21,12,27,15,44,18,10,19,25,6,45},
{5,22,12,27,15,45,18,11,19,26,6,45},
{5,23,12,28,15,46,18,12,19,27,6,45},
{5,23,12,28,15,46,18,12,19,27,6,45},
{5,23,12,29,15,47,18,14,19,28,6,46},
{5,23,12,29,15,47,18,14,19,28,6,46},
{5,23,12,29,15,47,18,14,19,28,6,46},
{5,24,12,30,15,48,18,15,19,29,6,46},
{5,24,12,30,15,48,18,15,19,29,6,46},
{5,24,12,30,15,49,18,16,19,30,6,46},
{5,24,12,30,15,49,18,16,19,30,6,46},
{5,24,12,31,15,50,18,17,19,31,6,46},
{5,24,12,31,15,50,18,17,19,31,6,46},
{5,24,12,31,15,51,18,18,19,32,6,46},
{5,24,12,31,15,51,18,18,19,32,6,46},
{5,24,12,31,15,52,18,19,19,33,6,46},
{5,24,12,31,15,52,18,19,19,33,6,46},
{5,24,12,31,15,52,18,19,19,33,6,46},
{5,24,12,32,15,52,18,20,19,34,6,46},
{5,24,12,32,15,52,18,20,19,34,6,46},
{5,25,12,33,15,53,18,21,19,35,6,46},
{5,25,12,33,15,53,18,21,19,35,6,46},
{5,25,12,33,15,53,18,21,19,35,6,46},
{5,25,12,33,15,53,18,21,19,35,6,46},
{5,25,12,33,15,54,18,22,19,35,6,45},
{5,25,12,33,15,54,18,22,19,35,6,45},
{5,24,12,33,15,54,18,22,19,36,6,45},
{5,24,12,33,15,54,18,22,19,36,6,45},
{5,23,12,33,15,54,18,23,19,36,6,44},
{5,23,12,33,15,54,18,23,19,36,6,44},
{5,23,12,33,15,55,18,24,19,37,6,43},
{5,23,12,33,15,55,18,24,19,37,6,43},
{5,22,12,33,15,55,18,25,19,37,6,42},
{5,22,12,33,15,55,18,25,19,37,6,42},
{5,22,12,33,15,55,18,25,19,38,6,42},
{5,22,12,33,15,55,18,25,19,38,6,42},
{5,21,12,33,15,55,18,26,19,38,6,41},
{5,21,12,33,15,55,18,26,19,38,6,41},
{5,21,12,33,15,55,18,26,19,38,6,41},
{5,21,12,33,15,55,18,26,19,38,6,41},
{5,21,12,33,15,55,18,27,19,39,6,40},
{5,21,12,33,15,55,18,27,19,39,6,40},
{5,20,12,33,15,55,18,27,19,39,6,39},
{5,20,12,33,15,55,18,27,19,39,6,39},
{5,18,12,32,15,54,18,28,19,39,6,37},
{5,18,12,32,15,54,18,28,19,39,6,37},
{5,18,12,32,15,54,18,28,19,40,6,37},
{5,18,12,32,15,54,18,28,19,40,6,37},
{5,17,12,32,15,54,18,28,19,40,6,36},
{5,16,12,31,15,54,18,29,19,40,6,35},
{5,16,12,31,15,54,18,29,19,40,6,35},
{5,15,12,31,15,53,18,29,19,40,6,34},
{5,15,12,31,15,53,18,29,19,40,6,34},
{5,13,12,30,15,53,18,29,19,40,6,32},
{5,13,12,30,15,53,18,29,19,40,6,32},
{5,13,12,30,15,53,18,29,19,40,6,32},
{5,13,12,32,15,52,18,30,19,40,6,30},
{5,13,12,32,15,52,18,30,19,40,6,30},
{5,11,12,30,15,51,18,30,19,41,6,28},
{5,11,12,30,15,51,18,30,19,41,6,28},
{5,11,12,30,15,51,18,30,19,41,6,28},
{5,9,12,28,15,51,18,30,19,41,6,26},
{5,9,12,28,15,51,18,30,19,41,6,26},
{5,8,12,26,15,50,18,30,19,41,6,25},
{5,8,12,26,15,50,18,30,19,41,6,25},
{5,7,12,25,15,49,18,30,19,41,6,25},
{5,7,12,25,15,49,18,30,19,41,6,25},
{5,7,12,25,15,48,18,31,19,41,6,23},
{5,7,12,25,15,48,18,31,19,41,6,23},
{5,5,12,23,15,47,18,31,19,41,6,22},
{5,5,12,23,15,47,18,31,19,41,6,22},
{5,4,12,22,15,46,18,31,19,42,6,20},
{5,4,12,22,15,46,18,31,19,42,6,20},
{5,2,12,20,15,45,18,32,19,42,6,18},
{5,2,12,20,15,45,18,32,19,42,6,18},
{5,1,12,18,15,44,18,32,19,42,6,18},
{5,1,12,18,15,44,18,32,19,42,6,18},
{5,0,12,18,15,43,18,32,19,42,6,17},
{5,0,12,18,15,43,18,32,19,42,6,17},
{4,58,12,17,15,40,18,32,19,43,6,17},
{4,57,12,23,15,40,18,32,19,43,6,16},
{4,57,12,23,15,40,18,32,19,43,6,16},
{4,55,12,22,15,38,18,32,19,43,6,14},
{4,55,12,22,15,38,18,32,19,43,6,14},
{4,55,12,22,15,38,18,32,19,43,6,14},
{4,53,12,21,15,37,18,32,19,44,6,12},
{4,53,12,21,15,37,18,32,19,44,6,12},
{4,52,12,21,15,36,18,32,19,44,6,11},
{4,52,12,21,15,36,18,32,19,44,6,11},
{4,50,12,20,15,34,18,32,19,44,6,9},
{4,50,12,20,15,34,18,32,19,44,6,9},
{4,49,12,20,15,33,18,32,19,45,6,8},
{4,49,12,20,15,33,18,32,19,45,6,8},
{4,47,12,19,15,32,18,32,19,45,6,7},
{4,47,12,19,15,32,18,32,19,45,6,7},
{4,46,12,19,15,30,18,33,19,45,6,6},
{4,46,12,19,15,30,18,33,19,45,6,6},
{4,44,12,18,15,29,18,33,19,46,6,5},
{4,44,12,18,15,29,18,33,19,46,6,5},
{4,43,12,18,15,27,18,33,19,46,6,4},
{4,43,12,18,15,27,18,33,19,46,6,4},
{4,42,12,18,15,26,18,33,19,47,6,3},
{4,41,12,17,15,24,18,33,19,47,6,2},
{4,41,12,17,15,24,18,33,19,47,6,2},
{4,40,12,17,15,23,18,34,19,48,6,1},
{4,40,12,17,15,23,18,34,19,48,6,1},
{4,40,12,17,15,25,18,34,19,48,6,1},
{4,38,12,16,15,26,18,34,19,49,5,59},
{4,38,12,16,15,26,18,34,19,49,5,59},
{4,38,12,16,15,26,18,34,19,49,5,59},
{4,38,12,16,15,26,18,34,19,49,5,59},
{4,38,12,16,15,26,18,34,19,49,5,59},
{4,36,12,16,15,28,18,36,19,50,5,57},
{4,36,12,16,15,28,18,36,19,50,5,57},
{4,36,12,16,15,29,18,36,19,50,5,57},
{4,36,12,16,15,29,18,36,19,50,5,57},
{4,34,12,15,15,30,18,36,19,51,5,56},
{4,34,12,15,15,30,18,36,19,51,5,56},
{4,33,12,15,15,31,18,36,19,52,5,55},
{4,33,12,15,15,31,18,36,19,52,5,55},
{4,31,12,15,15,32,18,37,19,53,5,55},
{4,31,12,15,15,32,18,37,19,53,5,55},
{4,31,12,15,15,32,18,37,19,53,5,55},
{4,30,12,15,15,33,18,37,19,53,5,54},
{4,30,12,15,15,33,18,37,19,53,5,54},
{4,29,12,15,15,34,18,38,19,54,5,54},
{4,29,12,15,15,34,18,38,19,54,5,54},
{4,29,12,15,15,35,18,39,19,55,5,54},
{4,29,12,15,15,35,18,39,19,55,5,54},
{4,28,12,15,15,36,18,39,19,56,5,53},
{4,28,12,15,15,36,18,39,19,56,5,53},
{4,27,12,16,15,37,18,40,19,56,5,53},
{4,27,12,16,15,37,18,40,19,56,5,53},
{4,27,12,16,15,38,18,40,19,57,5,53},
{4,27,12,16,15,38,18,40,19,57,5,53},
{4,27,12,16,15,39,18,41,19,58,5,53},
{4,27,12,16,15,39,18,41,19,58,5,53},
{4,27,12,16,15,39,18,41,19,58,5,53},
{4,26,12,16,15,40,18,41,19,29,5,53},
{4,26,12,16,15,40,18,41,19,29,5,53},
{4,26,12,17,15,41,18,42,20,0,5,52},
{4,26,12,17,15,41,18,43,20,1,5,52},
{4,26,12,17,15,41,18,43,20,1,5,52},
{4,25,12,17,15,42,18,43,20,2,5,52},
{4,25,12,17,15,42,18,43,20,2,5,52},
{4,25,12,17,15,43,18,44,20,2,5,52},
{4,25,12,17,15,43,18,44,20,2,5,52},
{4,25,12,18,15,44,18,45,20,3,5,52},
{4,25,12,18,15,44,18,45,20,3,5,52},
{4,25,12,18,15,44,18,45,20,4,5,52},
{4,25,12,18,15,44,18,45,20,4,5,52},
{4,25,12,18,15,44,18,45,20,4,5,52},
{4,25,12,19,15,45,18,46,20,5,5,52},
{4,25,12,19,15,45,18,46,20,5,5,52},
{4,25,12,19,15,46,18,46,20,5,5,53},
{4,25,12,19,15,46,18,46,20,5,5,53},
{4,25,12,19,15,46,18,46,20,5,5,53},
{4,26,12,20,15,46,18,47,20,6,5,54},
{4,26,12,20,15,46,18,47,20,6,5,54},
{4,26,12,20,15,47,18,48,20,6,5,54},
{4,26,12,20,15,47,18,48,20,6,5,54},
{4,27,12,21,15,47,18,48,20,7,5,54},
{4,27,12,21,15,47,18,48,20,7,5,54},
{4,27,12,21,15,48,18,49,20,7,5,55},
{4,27,12,21,15,48,18,49,20,7,5,55},
{4,27,12,21,15,48,18,49,20,8,5,55},
{4,27,12,22,15,49,18,49,20,8,5,56},
{4,27,12,22,15,49,18,49,20,8,5,56},
{4,28,12,22,15,49,18,49,20,9,5,56},
{4,28,12,22,15,49,18,49,20,9,5,56},
{4,28,12,22,15,49,18,49,20,9,5,56},
{4,29,12,23,15,49,18,49,20,9,5,57},
{4,29,12,23,15,49,18,49,20,9,5,57},
{4,29,12,23,15,49,18,49,20,9,5,57},
{4,30,12,23,15,49,18,49,20,9,5,57},
{4,30,12,23,15,49,18,49,20,9,5,57},
{4,31,12,24,15,50,18,50,20,9,5,58},
{4,31,12,24,15,50,18,50,20,9,5,58},
{4,31,12,24,15,50,18,50,20,9,5,58},
{4,31,12,24,15,50,18,50,20,9,5,58},
{4,31,12,24,15,50,18,50,20,9,5,58},
{4,32,12,24,15,49,18,49,20,9,5,59},
{4,32,12,24,15,49,18,49,20,9,5,59},
{4,33,12,25,15,49,18,49,20,9,6,0},
{4,33,12,25,15,49,18,49,20,9,6,0},
{4,33,12,25,15,49,18,49,20,8,6,0},
{4,33,12,25,15,49,18,49,20,8,6,0},
{4,34,12,25,15,49,18,49,20,8,6,0},
{4,34,12,25,15,49,18,49,20,8,6,0},
{4,35,12,25,15,48,18,48,20,7,6,1},
{4,35,12,25,15,48,18,48,20,7,6,1},
{4,36,12,25,15,48,18,48,20,7,6,1},
{4,36,12,25,15,48,18,48,20,7,6,1},
{4,37,12,25,15,47,18,47,20,6,6,2},
{4,37,12,25,15,47,18,47,20,6,6,2},
{4,37,12,25,15,47,18,47,20,6,6,2},
{4,37,12,25,15,47,18,47,20,6,6,2},
{4,38,12,25,15,46,18,46,20,5,6,3},
{4,38,12,25,15,46,18,46,20,5,6,3},
{4,39,12,25,15,45,18,45,20,4,6,3},
{4,39,12,25,15,43,18,43,20,3,6,3},
{4,39,12,25,15,43,18,43,20,3,6,3},
{4,40,12,25,15,43,18,43,20,3,6,4},
{4,40,12,25,15,43,18,43,20,3,6,4},
{4,41,12,25,15,41,18,41,20,2,6,4},
{4,41,12,25,15,41,18,41,20,2,6,4},
{4,42,12,25,15,40,18,40,20,1,6,5},
{4,42,12,25,15,40,18,40,20,1,6,5},
{4,43,12,25,15,39,18,39,20,0,6,5},
{4,43,12,25,15,39,18,39,20,0,6,5},
{4,43,12,24,15,37,18,37,19,59,6,5},
{4,43,12,24,15,37,18,37,19,59,6,5},
{4,43,12,24,15,36,18,36,19,58,6,6},
{4,43,12,24,15,36,18,36,19,58,6,6},
{4,44,12,24,15,34,18,34,19,56,6,6},
{4,44,12,24,15,34,18,34,19,56,6,6},
{4,45,12,23,15,32,18,32,19,55,6,6},
{4,45,12,23,15,32,18,32,19,55,6,6},
{4,45,12,23,15,32,18,32,19,55,6,6},
{4,45,12,23,15,30,18,30,19,54,6,6},
{4,45,12,23,15,30,18,30,19,54,6,6},
{4,45,12,22,15,29,18,29,19,53,6,6},
{4,45,12,22,15,29,18,29,19,53,6,6},
{4,46,12,22,15,30,18,30,19,51,6,7},
{4,46,12,22,15,30,18,30,19,51,6,7},
{4,47,12,21,15,30,18,30,19,50,6,7},
{4,47,12,21,15,30,18,30,19,50,6,7},
{4,47,12,21,15,30,18,30,19,50,6,7},
{4,47,12,20,15,31,18,31,19,48,6,7},
{4,47,12,20,15,31,18,31,19,48,6,7},
{4,48,12,20,15,31,18,31,19,46,6,7},
{4,48,12,20,15,31,18,31,19,46,6,7},
{4,48,12,19,15,32,18,32,19,44,6,7},
{4,48,12,19,15,32,18,31,19,43,6,8},
{4,48,12,19,15,32,18,31,19,43,6,8},
{4,48,12,18,15,32,18,30,19,42,6,8},
{4,48,12,18,15,32,18,30,19,42,6,8},
{4,48,12,18,15,32,18,30,19,42,6,8},
{4,49,12,17,15,32,18,28,19,40,6,8},
{4,49,12,17,15,32,18,28,19,40,6,8},
{4,49,12,17,15,32,18,28,19,40,6,8},
{4,49,12,16,15,32,18,26,19,39,6,8},
{4,49,12,16,15,32,18,26,19,39,6,8},
{4,49,12,16,15,32,18,26,19,39,6,8},
{4,49,12,15,15,32,18,24,19,37,6,8},
{4,49,12,15,15,32,18,24,19,37,6,8},
{4,49,12,15,15,32,18,24,19,37,6,8},
{4,49,12,14,15,32,18,22,19,36,6,7},
{4,49,12,14,15,32,18,22,19,36,6,7},
{4,49,12,13,15,32,18,21,19,34,6,7},
{4,49,12,13,15,31,18,20,19,33,6,7},
{4,49,12,13,15,31,18,20,19,33,6,7},
{4,49,12,12,15,31,18,19,19,33,6,7},
{4,49,12,12,15,31,18,18,19,31,6,7},
{4,49,12,12,15,31,18,18,19,31,6,7},
{4,49,12,11,15,31,18,17,19,30,6,7},
{4,49,12,11,15,30,18,16,19,28,6,7},
{4,49,12,11,15,30,18,16,19,28,6,7},
{4,49,12,10,15,30,18,15,19,27,6,7},
{4,49,12,10,15,30,18,14,19,25,6,8},
{4,49,12,10,15,30,18,14,19,25,6,8},
{4,49,12,9,15,30,18,12,19,22,6,8},
{4,49,12,9,15,30,18,12,19,22,6,8},
{4,50,12,9,15,30,18,11,19,21,6,8},
{4,50,12,8,15,30,18,10,19,20,6,8},
{4,50,12,8,15,30,18,10,19,20,6,8},
{4,50,12,8,15,29,18,9,19,18,6,8},
{4,50,12,7,15,29,18,8,19,17,6,8},
{4,50,12,7,15,29,18,8,19,17,6,8},
{4,50,12,7,15,28,18,7,19,16,6,8},
{4,50,12,7,15,28,18,7,19,16,6,8},
{4,50,12,6,15,28,18,6,19,15,6,8},
{4,50,12,6,15,28,18,6,19,15,6,8},
{4,50,12,6,15,27,18,5,19,14,6,8},
{4,50,12,6,15,27,18,5,19,14,6,8},
{4,50,12,5,15,27,18,4,19,13,6,9},
{4,50,12,5,15,27,18,4,19,13,6,9},
{4,50,12,5,15,27,18,3,19,12,6,9},
{4,50,12,5,15,27,18,3,19,12,6,9},
{4,50,12,4,15,26,18,1,19,11,6,9},
{4,50,12,4,15,26,18,1,19,11,6,9},
{4,51,12,4,15,26,18,0,19,10,6,10},
{4,51,12,4,15,26,18,0,19,10,6,10},
{4,51,12,4,15,25,17,59,19,9,6,10},
{4,51,12,4,15,25,17,59,19,9,6,10},
{4,51,12,3,15,25,17,57,19,8,6,10},
{4,51,12,3,15,25,17,57,19,8,6,10},
{4,51,12,3,15,25,17,57,19,8,6,10},
{4,52,12,3,15,24,17,56,19,7,6,11},
{4,52,12,3,15,24,17,56,19,7,6,11},
{4,52,12,3,15,24,17,56,19,7,6,11},
{4,52,12,3,15,24,17,55,19,6,6,12},
{4,52,12,3,15,24,17,55,19,6,6,12},
{4,52,12,3,15,24,17,55,19,6,6,12},
{4,53,12,3,15,24,17,54,19,6,6,13},
{4,53,12,3,15,24,17,54,19,5,6,13},
{4,53,12,3,15,24,17,54,19,5,6,13},
{4,53,12,3,15,23,17,53,19,5,6,14},
{4,53,12,3,15,23,17,53,19,5,6,14},
{4,54,12,3,15,23,17,53,19,4,6,15},
{4,54,12,3,15,23,17,53,19,4,6,15},
{4,54,12,3,15,23,17,52,19,4,6,15},
{4,54,12,3,15,23,17,52,19,4,6,15},
{4,55,12,3,15,23,17,51,19,4,6,16},
{4,55,12,3,15,23,17,51,19,4,6,16},
{4,55,12,3,15,23,17,51,19,4,6,16},
{4,55,12,3,15,23,17,51,19,4,6,17},
{4,55,12,3,15,23,17,51,19,4,6,17},
{4,56,12,4,15,23,17,51,19,5,6,18},
{4,56,12,4,15,23,17,51,19,5,6,18},
{4,57,12,4,15,24,17,51,19,5,6,19},
{4,57,12,4,15,24,17,51,19,5,6,19},
{4,57,12,4,15,24,17,51,19,5,6,19},
{4,58,12,5,15,24,17,51,19,6,6,20},
{4,58,12,5,15,24,17,51,19,6,6,20},
{4,59,12,5,15,24,17,50,19,6,6,21},
{4,59,12,5,15,24,17,50,19,6,6,21},
{5,0,12,6,15,25,17,51,19,6,6,22},
{5,0,12,6,15,25,17,51,19,6,6,22},
{5,0,12,6,15,25,17,51,19,6,6,22},
{5,1,12,7,15,25,17,51,19,6,6,23},
{5,1,12,7,15,25,17,51,19,6,6,23},
{5,1,12,7,15,25,17,51,19,6,6,23},
{5,2,12,8,15,25,17,52,19,6,6,24},
{5,2,12,8,15,25,17,52,19,6,6,26},
{5,2,12,8,15,25,17,52,19,6,6,26},
{5,3,12,9,15,26,17,52,19,7,6,27},
{5,3,12,9,15,26,17,52,19,7,6,27},
{5,5,12,10,15,26,17,53,19,8,6,28},
{5,5,12,10,15,26,17,53,19,8,6,28},
{5,6,12,11,15,27,17,54,19,8,6,28},
{5,6,12,11,15,27,17,54,19,8,6,28},
{5,6,12,11,15,27,17,54,19,8,6,28},
{5,7,12,12,15,28,17,54,19,9,6,31},
{5,7,12,12,15,28,17,54,19,9,6,31},
{5,8,12,13,15,29,17,55,19,10,6,31},
{5,8,12,13,15,29,17,55,19,10,6,31},
{5,9,12,14,15,29,17,56,19,11,6,32},
{5,9,12,14,15,29,17,56,19,11,6,32},
{5,10,12,15,15,30,17,57,19,12,6,34},
{5,10,12,15,15,30,17,57,19,12,6,34},
{5,11,12,16,15,30,17,58,19,13,6,35},
{5,11,12,16,15,30,17,58,19,13,6,35},
{5,12,12,17,15,31,17,59,19,14,6,36},
{5,12,12,17,15,31,17,59,19,14,6,36},
{5,13,12,18,15,32,18,0,19,15,6,37},
{5,13,12,18,15,32,18,0,19,15,6,37},
{5,14,12,19,15,33,18,1,19,17,6,38},
{5,14,12,19,15,33,18,1,19,17,6,38},
{5,15,12,20,15,34,18,2,19,18,6,39},
{5,15,12,20,15,34,18,2,19,18,6,39},
{5,16,12,21,15,35,18,3,19,19,6,40},
{5,16,12,21,15,35,18,3,19,19,6,40},
{5,17,12,22,15,38,18,4,19,20,6,41},
{5,17,12,22,15,38,18,4,19,20,6,41}};

  int FajarH ;
  int FajarM ;
  int ZuharH ;
  int ZuharM ;
  int AsarH ;
  int AsarM ;
  int MagribH ;
  int MagribM ;
  int IshaH ;
  int IshaM ;
  int UdayamH ;
  int UdayamM ;
  int Blink = 0;

  int FajarX = 34;
  int FajarY = 80;
  int UdayamX = 34;
  int UdayamY = 106;
  int ZuharX = 34;
  int ZuharY = 133;
  int AsarX = 146;
  int AsarY = 80;
  int MagribX = 146;
  int MagribY = 106;
  int IshaX = 146;
  int IshaY = 133;
  
  String fajar;
  String zuhar;
  String asar;
  String magrib;
  String isha;
  String udayam;
  
  int Prayer;
  int checking = 1;
  int Editi;
  
void setup()
{
  Serial.begin(115200);
  //myRTC.setDS1302Time(19, 20, 16, 1, 16, 6, 2020);
  WiFi.mode(WIFI_OFF);

  pinMode(button1, INPUT_PULLUP);
  pinMode(button2, INPUT_PULLUP);
  pinMode(BUZZER, OUTPUT);
  digitalWrite(BUZZER, LOW);

  //Set up the display
  tft.begin();
  tft.setOrientation(1);
  tft.setFont(Terminal6x8);
  tft.setBackgroundColor(COLOR_BLACK);
  tft.drawText(60,1,"Assalamu Alaikum",COLOR_GREEN);
  //tft.drawText(0,40,"---------",COLOR_CYAN);
  //tft.drawText(153,40,"--------",COLOR_CYAN);
  tft.drawText(40,60,"--------------------",COLOR_CYAN);
  tft.drawText(0,160,"-------------------------------",COLOR_CYAN);
  tft.drawText(70,167,"By Shajahan S",COLOR_GREEN); 
  tft.drawText(1,FajarY,"Faja",COLOR_WHITE);
  tft.drawText(1,UdayamY,"Uday",COLOR_WHITE);
  tft.drawText(1,ZuharY,"Zuha",COLOR_WHITE);  
  tft.drawText(110,AsarY,"Asar",COLOR_WHITE);
  tft.drawText(110,MagribY,"Magr",COLOR_WHITE);
  tft.drawText(110,IshaY,"Isha",COLOR_WHITE);
  tft.setFont(Terminal12x16);
  tft.drawText(35,45," Prayer Times ",COLOR_BLUE);
    
  ReadPrayerTime();
  Alarm.alarmRepeat(0,1,0, ReadPrayerTime);  // Midnight every day
}

bool debounce ()
{
  byte count = 0;
  for(byte i = 0; i < 5; i++)
  {
    if ( !digitalRead(button1) )
      count++;
    delay(10);
  }
 
  if(count > 2)  return 1;
  else           return 0;
}
 
void RTC_display()
{
  String ptext;
  ptext = Buty(day());
  ptext += "-";
  ptext += Buty(month());
  ptext += "-";
  ptext += year()-2000;
  
  tft.setFont(Terminal12x16);
  tft.drawText(2,19,ptext,COLOR_MAGENTA);
  ptext = Butyh(hour());
  ptext += ":";
  ptext += Buty(minute());
  ptext += ":";
  ptext += Buty(second());
  tft.drawText(120,19,ptext,COLOR_ORANGE);
}
 
byte edit(byte parameter)
{
  tft.setFont(Terminal12x16);
  static byte y_pos = 19,
              x_pos[5] = {2, 36, 73, 120, 152};
  Serial.print("In Edit Function Starting i and Param is:- ");
  Serial.print(Editi);
  Serial.print("-");
  Serial.println(parameter);
  int ii = Editi;
  int ip = parameter;
  char text[3];
  sprintf(text,"%02u", parameter);
 
  while( debounce() );   // call debounce function (wait for B1 to be released)
  unsigned long previous_m1 = millis();
  //while(millis() - previous_m1 < 3000) {
  //while(millis() - previous_m1 < 750 || (Editi != ii || ip != parameter)) {
  while(true){
    Serial.println("In While True Loop");
    while( !digitalRead(button2) ) {  // while B2 is pressed
      Serial.println("B2 button is pressed");
      parameter++;
      if(Editi == 0 && parameter > 31)    // if day > 31 ==> day = 1
        parameter = 1;
      if(Editi == 1 && parameter > 12)    // If month > 12 ==> month = 1
        parameter = 1;
      if(Editi == 2 && parameter > 99)    // If year > 99 ==> year = 0
        parameter = 0;
      if(Editi == 3 && parameter > 23)    // if hours > 23 ==> hours = 0
        parameter = 0;
      if(Editi == 4 && parameter > 59)    // if minutes > 59 ==> minutes = 0
        parameter = 0;
 
      sprintf(text,"%02u", parameter);
      tft.drawText(x_pos[Editi], y_pos,text,COLOR_ORANGE);
      delay(200);       // wait 200ms
    }
 
    tft.fillRectangle(x_pos[Editi], y_pos, x_pos[Editi]+27, y_pos+16, COLOR_BLACK);    unsigned long previous_m = millis();
    while( (millis() - previous_m < 250) && digitalRead(button1) && digitalRead(button2)) ;
    tft.drawText(x_pos[Editi], y_pos,text);
    previous_m = millis();
    while( (millis() - previous_m < 250) && digitalRead(button1) && digitalRead(button2)) ;
 
    if(!digitalRead(button1))
    {                     // if button B1 is pressed
      Editi = (Editi + 1) % 5;    // increment 'i' for the next parameter
      Serial.print("In Edit Function inside B1 pressed i is:- ");
      Serial.println(Editi);
      return parameter;   // return parameter value and exit
    }
    else
    {
      Serial.print("In Edit Function Else B1 pressed i is:- ");
      Serial.println(Editi);
      return parameter;   // return parameter value and exit      
    }
  }
  Editi = 0;
  Serial.print("In Edit Function outside While loop i is:- ");
  Serial.println(Editi);
}

void  loop(){  
  if( !digitalRead(button1) )  // if B1 is pressed
  if( debounce() )             // call debounce function (make sure B1 is pressed)
  {
    Serial.println("B1 button is pressed");
    Editi = 0;
    while( debounce() );  // call debounce function (wait for B1 to be released)

    Serial.println("In Day Edit");
    byte nday    = edit( day() );          // edit date
    Serial.print("New Day:- ");
    Serial.println(nday);
    Serial.println("In Month Edit");
    byte nmonth  = edit( month() );        // edit month
    Serial.print("New Month:- ");
    Serial.println(nmonth);
    Serial.println("In Year Edit");
    byte nyear   = edit( year() - 2000 );  // edit year
    Serial.print("New Year:- ");
    Serial.println(nyear);    
    Serial.println("In Hour Edit");
    byte nhour   = edit( hour() );         // edit hours
    Serial.print("New Hour:- ");
    Serial.println(nhour);    
    Serial.println("In Minute Edit");
    byte nminute = edit( minute() );       // edit minutes
    Serial.print("New Minute:- ");
    Serial.println(nminute);    
 
    // write time & date data to the RTC chip
    Serial.print("Saving time to RTC:- ");
    String newdate = String(nday);
    newdate += "-";
    newdate += nmonth;
    newdate += "-";
    newdate += nyear;
    newdate += " ";
    newdate += nhour;
    newdate += ":";
    newdate += nminute;
    
    Serial.print(newdate);
    myRTC.setDS1302Time(11, nminute, nhour, 1, nday, nmonth, nyear + 2000);
    myRTC.updateTime();
    //setTime(myRTC.hours, myRTC.minutes, myRTC.seconds, myRTC.dayofmonth, myRTC.month, myRTC.year); 
    setTime(nhour, nminute, 0, nday, nmonth, nyear + 2000);
    while(debounce());  // call debounce function (wait for button B1 to be released)
    //ReadPrayerTime();
  }
 
  RTC_display();    // diaplay time & calendar
  //digitalClockDisplay(); //Print Time in Serial port
  DisplayPrayerTimes(); //Print Prayer Times in Display

  //Blink = 0; //For testing disabled Blink
  if (Blink == 0)
  {
    Blink = 1;
    PrayerBlink();
  }
  else
  {
    //tft.fillRectangle(145, 19, 145+8, 19+12, COLOR_BLACK); 
    //tft.fillRectangle(180, 19, 180+8, 19+12, COLOR_BLACK); 
    //tft.fillRectangle(65, 35, 80, 8, COLOR_BLACK);
    Blink = 0;
  }
  Alarm.delay(1000);
}

String Butyh(int Dtime)
 {
  String Ntime;
  if (Dtime > 12)
  {
    Dtime = Dtime - 12;
  }
  if (Dtime < 10)
  {
    Ntime = "0";
    Ntime += String(Dtime);  
  }
  else
  {
    Ntime = String(Dtime);
  }
  return Ntime;
 }

String Buty(int Dtime)
 {
  String Ntime;
  if (Dtime < 10)
  {
    Ntime = "0";
    Ntime += String(Dtime);  
  }
  else
  {
    Ntime = String(Dtime);
  }
  return Ntime;
 }
 
void ReadPrayerTime(){
  myRTC.updateTime();
  setTime(myRTC.hours, myRTC.minutes, myRTC.seconds, myRTC.dayofmonth, myRTC.month, myRTC.year);
  int DayOfYear = calculateDayOfYear(day(), month());
  FajarH = Ptable[DayOfYear][0];
  FajarM = Ptable[DayOfYear][1];
  ZuharH = Ptable[DayOfYear][2];
  ZuharM = Ptable[DayOfYear][3];
  AsarH = Ptable[DayOfYear][4];
  AsarM = Ptable[DayOfYear][5];
  MagribH = Ptable[DayOfYear][6];
  MagribM = Ptable[DayOfYear][7];
  IshaH = Ptable[DayOfYear][8];
  IshaM = Ptable[DayOfYear][9];
  UdayamH = Ptable[DayOfYear][10];
  UdayamM = Ptable[DayOfYear][11];

  tft.setFont(Terminal6x8);
  tft.drawText(193,167,String(DayOfYear),COLOR_YELLOW);
  
  int PrayerHors[6] = {FajarH,UdayamH,ZuharH,AsarH,MagribH,IshaH};
  int PrayerMinutes[6] = {FajarM,UdayamM,ZuharM,AsarM,MagribM,IshaM};
  for (int i=0; i<6; i++)
  {
    int Cminutes = hour()*60+minute();
    int Pminutes = PrayerHors[i]*60+PrayerMinutes[i];
    if ( Cminutes < Pminutes )
     {
      Prayer = i;
      break;
     }
    Prayer = 5;
  }
  
  fajar = Butyh(FajarH) ;
  fajar += ":";
  fajar += Buty(FajarM);
  zuhar = Butyh(ZuharH);
  zuhar += ":";
  zuhar += Buty(ZuharM);
  asar = Butyh(AsarH);
  asar += ":";
  asar += Buty(AsarM);
  magrib = Butyh(MagribH);
  magrib += ":";
  magrib += Buty(MagribM);
  isha = Butyh(IshaH);
  isha += ":"; 
  isha += Buty(IshaM);
  udayam =  Butyh(UdayamH);
  udayam += ":";
  udayam += Buty(UdayamM);

  Alarm.alarmOnce(FajarH,FajarM,0, PlayAthan);
  Alarm.alarmOnce(ZuharH,ZuharM,0, PlayAthan);
  Alarm.alarmOnce(AsarH,AsarM,0, PlayAthan);
  Alarm.alarmOnce(MagribH,MagribM,0, PlayAthan);
  Alarm.alarmOnce(IshaH,IshaM,0, PlayAthan);
  Alarm.alarmOnce(UdayamH,UdayamM,0, PlayAthan);

  SetBlinkAlarm(1,FajarH,FajarM);
  SetBlinkAlarm(3,ZuharH,ZuharM);
  SetBlinkAlarm(4,AsarH,AsarM);
  SetBlinkAlarm(5,MagribH,MagribM);
  SetBlinkAlarm(2, UdayamH, UdayamM);

  Serial.println("Alarm: - Read Prayer Times for Today");   
  Serial.print("DayOfYear-");
  Serial.print(DayOfYear);
  Serial.print("-Fajar-");
  Serial.print(FajarH);
  Serial.print(":");
  Serial.print(FajarM);
  Serial.print("-Zuhar-");
  Serial.print(ZuharH);
  Serial.print(":");
  Serial.print(ZuharM);
  Serial.print("-Asar-");
  Serial.print(AsarH);
  Serial.print(":");
  Serial.print(AsarM);
  Serial.print("-Magrib-");
  Serial.print(MagribH);
  Serial.print(":");
  Serial.print(MagribM);
  Serial.print("-Isha-");
  Serial.print(IshaH);
  Serial.print(":");
  Serial.print(IshaM);
  Serial.print("-Udayam-");
  Serial.print(UdayamH);
  Serial.print(":");
  Serial.println(UdayamM);
 }

void SetPrayerF()
 {
  Prayer =  0;
 }
void SetPrayerU()
 {
  Prayer =  1;
 }
void SetPrayerZ()
 {
  Prayer =  2;
 }
void SetPrayerA()
 {
  Prayer =  3;
 }
void SetPrayerM()
 {
  Prayer =  4;
 }
void SetPrayerI()
 {
  Prayer =  5;
 }

void SetBlinkAlarm(int PrayerNum, int PrayerH, int PrayerM)
 {
  int BlinkM;
  int BlinkS;
  if (PrayerM > 58)
    {
      BlinkM = PrayerM; 
      BlinkS = 59;
    }
  else
    {
      BlinkM = PrayerM+1;
      BlinkS = 0;
    }
   if (PrayerNum == 0)
    Alarm.alarmOnce(PrayerH,BlinkM,BlinkS, SetPrayerF);
   if (PrayerNum == 1)
    Alarm.alarmOnce(PrayerH,BlinkM,BlinkS, SetPrayerU);
   if (PrayerNum == 2)
    Alarm.alarmOnce(PrayerH,BlinkM,BlinkS, SetPrayerZ);
   if (PrayerNum == 3)
    Alarm.alarmOnce(PrayerH,BlinkM,BlinkS, SetPrayerA);   
   if (PrayerNum == 4)
    Alarm.alarmOnce(PrayerH,BlinkM,BlinkS, SetPrayerM);
   if (PrayerNum == 5)
    Alarm.alarmOnce(PrayerH,BlinkM,BlinkS, SetPrayerI);
 }
   
void DisplayPrayerTimes()
{
  tft.setFont(Terminal12x16); 
  tft.drawText(FajarX, FajarY,fajar,COLOR_YELLOW);
  tft.drawText(UdayamX, UdayamY,udayam,COLOR_YELLOW);
  tft.drawText(ZuharX, ZuharY, zuhar,COLOR_YELLOW);
  
  tft.drawText(AsarX, AsarY,asar,COLOR_YELLOW);
  tft.drawText(MagribX, MagribY,magrib,COLOR_YELLOW);
  tft.drawText(IshaX, IshaY,isha,COLOR_YELLOW);
}

void PrayerBlink()
{
  int BlinkX;
  int BlinkY;
 if (Prayer == 0)
  {
    BlinkX = FajarX;
    BlinkY = FajarY;
  }
 if (Prayer == 1)
  {
    BlinkX = UdayamX;
    BlinkY = UdayamY;
  }
 if (Prayer == 2)
  {
    BlinkX = ZuharX;
    BlinkY = ZuharY;
  }
 if (Prayer == 3)
  {
    BlinkX = AsarX;
    BlinkY = AsarY;
  }
 if (Prayer == 4)
  {
    BlinkX = MagribX;
    BlinkY = MagribY;
  }
 if (Prayer == 5)
  {
    BlinkX = IshaX;
    BlinkY = IshaY;
  }
 tft.fillRectangle(BlinkX, BlinkY, BlinkX+60, BlinkY+16, COLOR_BLACK);
}

int calculateDayOfYear(int day, int month) {
  
  int daysInMonth[] = {31,29,31,30,31,30,31,31,30,31,30,31};
  
  if (day < 1) 
  {
    return 999;
  } else if (day > daysInMonth[month-1]) {
    return 999;
  }
  
  int doy = 0;
  for (int i = 0; i < month - 1; i++) {
    doy += daysInMonth[i];
  }
  
  doy += day;
  return doy - 1;
}

void PlayAthan()
 {
  digitalWrite(BUZZER, HIGH);
  Serial.print("Playing Athan Here");
  delay(500);
  digitalWrite(BUZZER, LOW);
 }
 
void digitalClockDisplay()
{
  if ( second() == 1 )
  {
    //Serial.print("Value of checking:- ");
    //Serial.println(checking);
    //if ( checking == 1 )
    //{
     // Serial.println("Loop for High");
     // digitalWrite(BUZZER, HIGH);
     // checking = 2;
    //}
    //else
    //{
    //  Serial.println("Loop for Low");
     // checking = 1;
     // digitalWrite(BUZZER, LOW); 
    //}
    
  //myRTC.updateTime();
  Serial.print(hour());
  printDigits(minute());
  printDigits(second());
  Serial.print(" ");
  Serial.print(day());
  Serial.print("/");
  Serial.print(month());
  Serial.print("/");
  Serial.print(year());
  Serial.print("==");
  Serial.print(myRTC.dayofmonth);
  Serial.print("/");
  Serial.print(myRTC.month);
  Serial.print("/");
  Serial.print(myRTC.year);
  Serial.print("-");
  Serial.print(myRTC.hours);
  Serial.print(":");
  Serial.print(myRTC.minutes);
  Serial.print(":");
  Serial.print(myRTC.seconds);
  Serial.println();
  } 
}

void printDigits(int digits)
{
  Serial.print(":");
  if(digits < 10)
    Serial.print('0');
  Serial.print(digits);
}
