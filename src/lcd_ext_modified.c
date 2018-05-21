/*****************************************************************************
 File Name            : lcd_ext_modified.c
 Title                : LCD Utilities
 Date                 : 04/09/2018
 Version              : 1.0
 Target MCU           : ATmega128
 Author               : Wilmer Suarez
 DESCRIPTION
 The file containsaa function that makes it easier for a C
 program to use the LCD display.           
 The function putchar() puts a single character, passed to it as an argument,
 into the display buffer at the position corresponding to the value of
 variable index. This putchar function replaces the standard putchar funtion,
 so a printf statement will print to the LCD.  
****************************************************************************/
#include "header.h"     // Includes the ATmega128 Definitions, Macros, and Instinsic functions
#include "lcd.h"

// -- Function Prototypes -- //
int putchar(int);
void backspace();
void formFeed();
void newline();
void carriageReturn();
void charPut();

static char index;    // index into display buffer

/***********************************************************************
 Function             : int putchar(int c)
 Date                 : 02/18/2018
 Version              : 2.0
 Target MCU           : ATmega128
 Author               : Wilmer Suarez 
 DESCRIPTION
 This function displays a single ascii chararacter c on the lcd at the
 position specified by the global variable index
 NOTE: update_lcd_dog() function must be called after to see results
 
 Modification:
 Functionality for 4 types of escape sequences:
 \b, \f, \n, & \r
***********************************************************************/
int putchar(int c) {
  switch(c) {
    case '\b':  
      backspace();
      break;
    case '\f':
      formFeed();
      break;
    case '\n':
      newline();
      break;
    case '\r':
      carriageReturn();
      break;
    default:
      charPut(c);
      break;
  }
  return c;
}

/*********************************************
 Function             : void backspace()
 Date                 : 02/19/18
 Version              : 1.0
 Target MCU           : ATmega128
 Author               : Wilmer Suarez
 DESCRIPTION
 Moves display one position backwards. 
*********************************************/
void backspace() {
  if(index == 0) {
    // Do nothing
  } else {
    index--;
  }
}

/*****************************************************
 Function             : void formFeed()
 Date                 : 02/19/18
 Version              : 1.0
 Target MCU           : ATmega128
 Author               : Wilmer Suarez
 DESCRIPTION
 Clear display and go back to beginning of first line. 
*****************************************************/
void formFeed() {
  for(char i = 0; i < 16; i++)
    dsp_buff_1[i] = ' ';
  
  for(char i = 0; i < 16; i++)
    dsp_buff_2[i] = ' ';
  
  for(char i = 0; i < 16; i++)
    dsp_buff_3[i] = ' ';  
  
  index = 0;
}

/***************************************
 Function             : void newline()
 Date                 : 02/19/18
 Version              : 1.0
 Target MCU           : ATmega128
 Author               : Wilmer Suarez
 DESCRIPTION
 Go to the beginning of the next line.  
***************************************/
void newline() {
  if (index < 16) {
    index = 16;     // If on first line, go to beginning of the second line
  }
  else if (index < 32) {
    index = 32;     // If on second line, go to beginning of the third line
  }
  else if (index < 48) {
    index = 0;      // If on third line, go to beginning of the first line
  }
}

/********************************************
 Function             : void carriageReturn()
 Date                 : 02/19/18
 Version              : 1.0
 Target MCU           : ATmega128
 Author               : Wilmer Suarez
 DESCRIPTION
 Goes to the beginning of the current line. 
********************************************/
void carriageReturn() {
  if (index < 16) {
    index = 0;      // If on first line, go to beginning of first line
  }
  else if (index < 32) {
    index = 16;     // If on second line, go to beginning of second line
  }
  else if (index < 48) {
    index = 32;     // If on third line, go to beginning of third line
  }
}

/************************************************
 Function             : void charPut()
 Date                 : 02/19/18
 Version              : 1.0
 Target MCU           : ATmega128
 Author               : Wilmer Suarez
 DESCRIPTION
 Normal Printing if no escape sequence is present. 
*************************************************/
void charPut(int c) {
  if (index < 16) {
    dsp_buff_1[index++] = (char)c;          // Print character on first line of LCD
  }
  else if (index < 32) {
    dsp_buff_2[index++ - 16] = (char)c;     // Print character on second line of LCD
  }
  else if (index < 48) {
    dsp_buff_3[index++ - 32] = (char)c;     // Print character on third line of LCD
  }
  else {
    index = 0;                              // Reset printing to the first line
    dsp_buff_1[index++] = (char)c;
  }
}
  