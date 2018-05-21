//***********************************************************************
// File Name            : "lcd_dog_iar_driver.c" 
// Title                : lcd_dog_iar_driver
// Date                 : 04/09/2018  
// Version              : 1.0 
// Target MCU           : ATMEGA128A
// Author               : Wilmer Suarez 
// DESCRIPTION 
// This module contains procedures to initialize and update
// DOG text based LCD display modules, including the EA DOG163M LCD
// modules configured with three (3) 16 charactors display lines.
//
// The display module hardware interface uses a 1-direction, write only
// SPI interface.
//
// The display module software interface uses three (3) 16-byte
// data (RAM) based display buffers - One for each line of the display.  
//***********************************************************************  
#include "header.h"     // Includes the ATmega128 Definitions, Macros, and Instinsic functions

// Declare external function prototypes
void init_lcd_dog();
void update_lcd_dog();
// Declare local function prototypes
void init_spi_lcd();
void lcd_spi_transmit_CMD(char command);
void lcd_spi_transmit_DATA(char data);

// Define SPI features names
#define	SCK     1
#define	MISO    3
#define	MOSI    2
#define	SS_bar  0
#define	RS      4
#define	BLC     5
#define FREQ    16      // Clock Speed (MHz)   

//--------------- Display buffer definitions ---------------//
char dsp_buff_1[16];    
char dsp_buff_2[16];
char dsp_buff_3[16];

//*************************************************
// Function Name        : "init_lcd_dog" 
// Date                 : 02/24/2018
// Version              : 1.0 
// Target MCU           : ATMEGA128A 
// Author               : Wilmer Suarez
// DESCRIPTION 
// Initializes DOG module LCD display for SPI   
// operation. 
//
// Warnings             : none 
// Restrictions         : none 
// Algorithms           : none 
// References           : none 
// 
// Revision History     : Initial version  
//*************************************************
void init_lcd_dog() {
//--------------- Initialize LCD DOG ---------------//
  init_spi_lcd();
  
//--------------- Delay for 40ms ---------------//
  __delay_cycles(FREQ * 40000);

//--------------- Function Set 1 ---------------//
  char command = 0x39;                  // Command_1
  lcd_spi_transmit_CMD(command);

  __delay_cycles(FREQ * 30);      // Delay for 30us
   
//--------------- Function Set 2 ---------------//
  lcd_spi_transmit_CMD(command);

  __delay_cycles(FREQ * 30);      // Delay for 30us
   
//--------------- Bias Set ---------------//
  command = 0x1E;                       // Set bias value
  lcd_spi_transmit_CMD(command);

  __delay_cycles(FREQ * 30);      // Delay for 30us
   
//--------------- Constrast Set ---------------//
  command = 0x77;                       // LCD Brightness
  lcd_spi_transmit_CMD(command);

  __delay_cycles(FREQ * 30);      // Delay for 30us
   
//--------------- Power Control ---------------//
  command = 0x50;      
  lcd_spi_transmit_CMD(command);
   
  __delay_cycles(FREQ * 30);      // Delay for 30us
  
//--------------- Follower Control ---------------//
  command = 0x6C;                       // Follower mode ON
  lcd_spi_transmit_CMD(command);

  __delay_cycles(FREQ * 200000);        // Delay for 200ms (For Power Stability)
    
//--------------- Display On ---------------//
  command = 0x0C;                       // Display ON, Cursor OFF, Blink OFF
  lcd_spi_transmit_CMD(command);
   
  __delay_cycles(FREQ * 30);      // Delay for 30us
   
//--------------- Clear Display ---------------//  
  command = 0x01;                       // Clear display, cursor home
  lcd_spi_transmit_CMD(command);
  
  __delay_cycles(FREQ * 2000);          // Delay for 2ms
   
//--------------- Entry Mode ---------------//  
  command = 0x06;                       // Clear display, cursor home
  lcd_spi_transmit_CMD(command);

  __delay_cycles(FREQ * 30);      // Delay for 30us
  
}

//*************************************************
// Function Name        : "init_spi_lcd" 
// Date                 : 02/24/2018
// Version              : 1.0 
// Target MCU           : ATMEGA128A 
// Author               : Wilmer Suarez
// DESCRIPTION 
// Initializes SPI port for command and data writes
// to LCD via SPI.
//
// Warnings             : none 
// Restrictions         : none 
// Algorithms           : none 
// References           : none 
// 
// Revision History     : Initial version  
//*************************************************

void init_spi_lcd() {
   // Enables SPI, Master, SCLK Freq: fosc/64
   SPCR = ((1 << SPE) | (1 << MSTR) | (1 << CPOL) | (1 << CPHA) | (1 << SPR1) | (1 << SPR0));
  
   // Kill spurious data
   // Reading SPSR and SPDR, clears the SPIF bit
   TESTBIT(SPSR, SPIF);
   TESTBIT(SPDR, 0);
}

//*************************************************
// Function Name        : "lcd_spi_transmit_CMD" 
// Date                 : 02/24/2018
// Version              : 1.0 
// Target MCU           : ATMEGA128A 
// Author               : Wilmer Suarez
// DESCRIPTION 
// Outputs the byte passed via SPI port. Waits for
// data to be written by SPI port before continuing.
//
// Warnings             : none 
// Restrictions         : none 
// Algorithms           : none 
// References           : none 
// 
// Revision History     : Initial version  
//*************************************************

void lcd_spi_transmit_CMD(char command) {
  CLEARBIT(PORTB, RS);          // RS = 0 = command
  CLEARBIT(PORTB, SS_bar);      // /SS = slave selected

  SPDR = command;               // Write data to SPI port

  // Wait for transmission complete
  while(!(SPSR & (1 << SPIF)));
  
  // Clear SPIF 
  TESTBIT(SPSR, SPIF);
  TESTBIT(SPDR, 0);
  
  // De-select slave
  SETBIT(PORTB, SS_bar);
}

//*************************************************
// Function Name        : "update_lcd_dog" 
// Date                 : 02/24/2018
// Version              : 1.0 
// Target MCU           : ATMEGA128A 
// Author               : Wilmer Suarez
// DESCRIPTION 
// Updates all 3 lines of the LCD using the contents
// of dsp_buff_1, dsp_buff_2, dsp_buff_3
//
// Warnings             : none 
// Restrictions         : none 
// Algorithms           : none 
// References           : none 
// 
// Revision History     : Initial version  
//*************************************************

void update_lcd_dog() {
//--------------- Initialize LCD DOG ---------------//
  init_spi_lcd();
  
  char charCount = 16;         // Number of characters per line 
  char *pLine1 = dsp_buff_1;   // Pointer to beginning of display buffer 1
  char *pLine2 = dsp_buff_2;   // Pointer to beginning of display buffer 2
  char *pLine3 = dsp_buff_3;   // Pointer to beginning of display buffer 3

 //--------------- Send line 1 to the LCD ---------------//
  // Send DDRAM Address
  char ddram_Addr = 0x80;                 // Init DDRAM Addr-Ctr
  lcd_spi_transmit_CMD(ddram_Addr);
  __delay_cycles(FREQ * 30);        // Delay for 30us
  // Send dsp_buff_1
  while(charCount != 0) {
    lcd_spi_transmit_DATA(*pLine1++);       // Send byte to LCD
    __delay_cycles(FREQ * 30);      // Delay for 30us
    charCount--;
  }
  
//--------------- Send line 2 to the LCD ---------------//
  charCount = 16;                         // Reinitialize count
  // Send DDRAM Address
  ddram_Addr = 0x90;                      // Init DDRAM Addr-Ctr
  lcd_spi_transmit_CMD(ddram_Addr);
  __delay_cycles(FREQ * 30);        // Delay for 30us
  // Send dsp_buff_2
  while(charCount != 0) {
    lcd_spi_transmit_DATA(*pLine2++);       // Send byte to LCD
    __delay_cycles(FREQ * 30);      // Delay for 30us
    charCount--;
  }
//--------------- Send line 3 to the LCD ---------------//
  charCount = 16;                         // Reinitialize count
  // Send DDRAM Address
  ddram_Addr = 0xA0;                      // Init DDRAM Addr-Ctr
  lcd_spi_transmit_CMD(ddram_Addr);
  __delay_cycles(FREQ * 30);        // Delay for 30us
  // Send dsp_buff_3
  while(charCount != 0) {
    lcd_spi_transmit_DATA(*pLine3++);       // Send byte to LCD
    __delay_cycles(FREQ * 30);      // Delay for 30us
    charCount--;
  }
}

//*************************************************
// Function Name        : "lcd_spi_transmit_DATA" 
// Date                 : 02/24/2018
// Version              : 1.0 
// Target MCU           : ATMEGA128A 
// Author               : Wilmer Suarez
// DESCRIPTION 
// Outputs the byte passed via SPI port. Waits for
// data to be written by SPI port before continuing.
//
// Warnings             : none 
// Restrictions         : none 
// Algorithms           : none 
// References           : none 
// 
// Revision History     : Initial version  
//*************************************************

void lcd_spi_transmit_DATA(char data) {
  SETBIT(PORTB, RS);            // RS = 1 = data
  CLEARBIT(PORTB, SS_bar);      // /SS = slave selected

  SPDR = data;                  // Write data to SPI port

  // Wait for transmission complete
  while(!(SPSR & (1 << SPIF)));
  
  // Clear SPIF 
  TESTBIT(SPSR, SPIF);        
  TESTBIT(SPDR, 0);
    
  // De-select slave
  SETBIT(PORTB, SS_bar);
}