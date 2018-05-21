/*********************************************************************
  File Name            : "Display_Time_Temp_Hum_FSM.c" 
  Title                : Table Driven FSM (Display Time, Temp, & Hum)
  Date                 : 04/09/2018  
  Version              : 1.0 
  Target MCU           : ATMEGA128A
  Author               : Wilmer Suarez 
  DESCRIPTION 
  This module is used to communicate with the DS1306 RTC and 
  HumidIcon to display:
  - Time, in a 24 hour format 
  - Temperature and Humidity 
  on an the LCD screen. 
  Display is updated every second.
  The Time and Alarm 0 of the DS1306 can be changed by the user,
  through the 4x4 Keypad.
  This is implemented using a Table Driven FSM.
*********************************************************************/
  
// ----- Include Files ----- //
#include "header.h"     // Includes the ATmega128 Definitions, Macros, and Intrinsic functions
#include "DS1306.h"
#include "FSM.h"

// Gloabl varaible that holds the present state of the FSM
state present_state = idle;

// Initially display celcius
bool tempCF = true;

// ----- Local Function Prototypes ----- //
void check_release();
void init_ADC();

// PortB pin numbers for columns and rows of the keypad
#define COL1  7   
#define COL2  6
#define COL3  5
#define COL4  4
#define ROW1  3
#define ROW2  2
#define ROW3  1
#define ROW4  0
   
// Key table
const key kTable[16] =  {del, co2, zero, tempChange, back, nine, eight, seven, setAlarm0, six, five, four, setTime, three, two, one};

/****************************************************
  ISR Name             : __interrupt void ISR_INT0()
  Target MCU           : ATmega128A
  Author               : Wilmer Suarez
  Version              : 2.0
  DESCRIPTION
  Interrupt service routine for INT0.
  Occurs when a key is pressed.
****************************************************/
#pragma vector=INT0_vect              // Vector Location for INT0 interrupt
__interrupt void ISR_INT0() {
  char keycode;                       // Holds key table position
  key keypressed;                     // Holds key type value
  
  if(!TESTBIT(PINC,ROW1))             // Find Row of pressed key
    keycode = 0;
  else if(!TESTBIT(PINC,ROW2))
    keycode = 4;
  else if(!TESTBIT(PINC,ROW3))
    keycode = 8;
  else if(!TESTBIT(PINC,ROW4))
    keycode = 12;
  
  DDRC = 0x0F;                        // Reconfigure PORTC for Columns
  PORTC = 0xF0;
  
  __delay_cycles(256);                // Let PORTC settle
  
  if(!TESTBIT(PINC,COL1))             // Find Column
    keycode += 0;
  else if(!TESTBIT(PINC,COL2))
    keycode += 1;
  else if(!TESTBIT(PINC,COL3))
    keycode += 2;
  else if(!TESTBIT(PINC,COL4))
    keycode += 3;
  
  DDRC = 0xF0;                        // Reconfigure PORTC for Rows for next keypad press
  PORTC = 0x0F;
  
  keypressed = (kTable[keycode]);     // Get key value from table 
  check_release();                    // Wait for keypad release.
  
  // FSM called 
  if(keypressed != tempChange) {
    // ---------- FSM Call ---------- //
    fsm(present_state, keypressed);   // Execute function associated with the keypressed variable
                                      // and update the present
  } else {
    tempCF = !tempCF;
  }

  // Disable INT1 when present_state is not idle
  if(present_state != idle) {
    EIMSK = 0x05;                 
  } else {
    EIMSK = 0x07;             
  }
}

/****************************************************
  ISR Name             : __interrupt void ISR_INT2()
  Target MCU           : ATmega128A
  Author               : Wilmer Suarez
  Version              : 1.0
  DESCRIPTION
  Interrupt service routine for INT2.
  Occurs when the active low interrupt of 
  the DS1306.
****************************************************/
#pragma vector=INT2_vect        // Vector Location for INT2 interrupt
__interrupt void ISR_INT2() {
  CLEARBIT(PORTA, 2);           // Set PORTA test Pin
  read_RTC(0x07);               // Clear IRQF0 (Interrupt 0 Request Flag)
}

// -------------------------- Main -------------------------- //
int main() {
  // -------------------------- PORT Configuration -------------------------- //
  // Port A Configurations for Humidicon and RTC Slave Select
  DDRA = 0x07;                      // Pin 0, 1, & 2: Outputs (Chip Select for humidicon and RTC, & INT2 test Pin) 
  SETBIT(PORTA, 0);                 // Initially de-select Humidicon 
  CLEARBIT(PORTA, 1);               // Initially de-select DS1306 RTC
  
  // Port C Configurations for keypad (initial configuration)
  DDRC = 0xF0;                      // High nibble outputs, low nibble inputs 
  PORTC = 0x0F;                     // High nibble outputs 0's initially

  // Port B Configurations for SPI
  DDRB = (1 << DDB0) | (1 << DDB1) | (1 << DDB2) | (1 << DDB4);         // SCK, /SS, RS, MOSI: Output, MISO: Input, 
  SETBIT(PORTB, 0);                                                     // Initially de-select LCD 
  
  // -------------------------- PORTD & Interrupt Configuration -------------------------- //
  DDRD = 0xF8;                      // INT0, INT1, INT2 Input
  PORTD = 0x01;                     // INT0 pullup enabled
  MCUCR = 0x30;                     // Sleep enabled for power down mode.
  EIMSK = 0x03;                     // Enable interrupt INT0, INT1, and INT2.
                                    // Sense control (@EICRA) is low by default
  
  // --------------- Initialize ADC --------------- //
  init_ADC();
  
  // ------------------------------ DS1306 interrupt Configuration ------------------------------ //
  // Initial Configuration for DS1306's alarm 0 and interrupt 0
  DS1306_RTC_config();
  
  __enable_interrupt();             // Enable global interrutps
  
  while(1) {
    // Continuously wait for interrupt (every 1 second (1Hz))
  }
}

/*******************************************
  Function             : void init_ADC()
  Target MCU           : ATmega128 @ 16MHz
  Author               : Wilmer Suarez
  Version              : 1.0
  DESCRIPTION
  Initialize ADC 
*******************************************/
void init_ADC() {
  ADMUX = 0xC3;         // ADC3, Internal 2.56V as Reference
  ADCSRA = 0x87;        // Enable ADC, Set Prescaler to 128 (125kHz)
}

/******************************************************
  Function             : void DS1306_RTC_config()
  Target MCU           : ATmega128 @ 16MHz
  Author               : Wilmer Suarez
  Version              : 1.0
  DESCRIPTION
  This function checks if the keypad has been released 
  and not bouncing.
******************************************************/
void check_release(void) {
  while(!TESTBIT(PIND,0));     // Check that keypad key is released.
  
  __delay_cycles(50000);       // Delay (.05secs) / (1 / 1MHz) cycles.
  
  while(!TESTBIT(PIND,0));     // Check that key has stopped bouncing.
}
