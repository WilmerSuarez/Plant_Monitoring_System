/******************************************************************
 File Name            : "DS1306_RTC_drivers.c"
 Title                : DS1306 Driver & Test Functions 
 Date                 : 04/09/2018  
 Version              : 1.0
 Target MCU           : ATmega128
 Author               : Wilmer Suarez
 DESCRIPTION
 This module defines the driver and configuration functions needed 
 to allow the Microcontroller to communicate with the DS1306
 and display the time, in 24-hour format, on the LCD. 
******************************************************************/

// ----- Include Files ----- //
#include "header.h"     // Includes the ATmega128 Definitions, Macros, and Instinsic functions
#include "DS1306.h"
#include "lcd.h"
#include "humidicon.h"
#include <stdio.h>

// ------- Static function Prototypes ------- //
static void write_RTC(unsigned char reg_RTC, unsigned char data_RTC);
static void block_read_RTC(volatile unsigned char *array_ptr, unsigned char start_addr, unsigned char count);

// ----- Global variables and arrays ----- //
volatile unsigned char RTC_time_date_write[3] = {0x00, 0x00, 0x00};  // Holds the initial data to be written to the DS1306 time registers
volatile unsigned char RTC_time_date_read[3];                        // Holds the data read from the DS1306 time registers
volatile unsigned char alarm0_config[4] = {0x80, 0x80, 0x80, 0x80};  // Holds data to configure alarm 0 to cause an interrupt each second
unsigned char data;                                                  // Holds current byte of data read from the DS1306
volatile unsigned char *arrPtr;                                      // Points to current array

/*************************************************************
 ISR Name             : void display_time_RTC()
 Target MCU           : ATmega128 @ 16MHz
 Date                 : 04/09/2018
 Author               : Wilmer Suarez
 Version              : 1.0
 DESCRIPTION
 This Interrupt Service Routine reads the hours, minutes, and 
 seconds register of the DS1306, converts the BCD value to 
 integer, and displays it on the LCD.
 This interrupt occurs every second.
*************************************************************/
#pragma vector=INT1_vect                        // Vector Location for INT1 interrupt
__interrupt void display_time_ISR() {
  // Variables
  unsigned char readAddr = 0x00, count0 = 3;
  unsigned int hours, minutes, seconds;
  
  // ------------------------------ SPI Configuration ------------------------------ //
  // Configure Microcontroller SPI to communicate with the DS1306 RTC
  SPI_rtc_DS1306_config();
  
  // -------- Read the DS1306's Time and Date registers -------- //
  arrPtr = RTC_time_date_read;                  // Pointing to start of read Array
  block_read_RTC(arrPtr, readAddr, count0);     // Read Time registers
  
  // ----- Convert Hours, Minutes, and Seconds from BCD to Integer ----- //
  hours = (((RTC_time_date_read[2] & 0xF0) >> 4) * 10);
  hours += RTC_time_date_read[2] & 0x0F;
  minutes = (((RTC_time_date_read[1] & 0xF0) >> 4) * 10);
  minutes += RTC_time_date_read[1] & 0x0F;
  seconds = (((RTC_time_date_read[0] & 0xF0) >> 4) * 10);
  seconds += RTC_time_date_read[0] & 0x0F;
  
  // -------------------- Display Time, Temp, & Hum -------------------- //
  printf("\fTime: %02d:%02d:%02d\n", hours, minutes, seconds);
  
  meas_display_rh_temp();           // Reads, calculates, and displays Temp and Hum
}

/***************************************************************
 Function             : void DS1306_RTC_config()
 Date                 : 04/09/2018
 Version              : 1.0
 Target MCU           : ATmega128
 Author               : Wilmer Suarez
 DESCRIPTION
 This function intializes the DS1306's control register by 
 clearing the write protect bit and enabling the 1hz output, 
 and alarm 0 intterupt. 
 It also sets up the alarm 0 to cause an interrupt every second
***************************************************************/
void DS1306_RTC_config() {
  // Variables
  unsigned char writeAddr = 0x80, alarm0Addr = 0x87, count0 = 3, count1 = 4;
  
  // ------------------------------ SPI Configuration ------------------------------ //
  // Configure Microcontroller SPI to communicate with the DS1306 RTC
  SPI_rtc_DS1306_config();
  
  // ----------------------- Setup DS1306's Control register ----------------------- //
  // Clear Write Protect bit. It is intially undefined 
  write_RTC(0x8F, 0x00);                        // Two writes needed because if wp is set, writing can't be done to any other bit.
  write_RTC(0x8F, 0x05);                        // Enable 1Hz output and Enable Alarm 0 (AIE0) to allow /INT0 to be asserted.

  // ----------------- Initialize DS1306's Time registers ----------------- //
  arrPtr = RTC_time_date_write;                 // Pointing to start of write Array
  block_write_RTC(arrPtr, writeAddr, count0);
  
  // --------------------- Initialize DS1306's Alarm 0 to Interrupt every second --------------------- //
  arrPtr = alarm0_config;                       // Pointing to start of alarm0_config Array
  block_write_RTC(arrPtr, alarm0Addr, count1);
}

/************************************************************
 Function             : void SPI_rtc_ds1306_config()
 Date                 : 04/09/2018
 Version              : 1.0
 Target MCU           : ATmega128
 Author               : Wilmer Suarez
 DESCRIPTION
 This function configures an ATmega128 operated at 16 MHz to 
 communicate with the DS1306 by SPI.  
 SCLK is operated a the maximum possible frequency for 
 the DS1306 (2MHz).
************************************************************/
void SPI_rtc_DS1306_config() {  
  // --------------------- SPI Configuration --------------------- //
  // Set SPE, MSTR, CPOL, CPHA, SPR0, and SPI2X //
  // Pre-scalar = 8 -> SCLK = 2MHz //
  SPCR = (1 << SPE) | (1 << MSTR) | (1 << CPOL) | (1 << CPHA) | (1 << SPR0);
  SPSR = (1 << SPI2X);
  
  // --- Clear SPIF in SPSR --- //
  TESTBIT(SPSR, SPIF);
  TESTBIT(SPDR, 0);
}

/*************************************************************************************
 Function             : void write_RTC (unsigned char reg_RTC, unsigned char data_RTC)
 Date                 : 04/09/2018
 Version              : 1.0
 Target MCU           : ATmega128
 Author               : Wilmer Suarez
 DESCRIPTION
 This function writes data to a register in the RTC. 
 To accomplish this, it must first write the register's address 
 (reg_RTC) followed by writing the data (data_RTC). 
 In the DS1306 data sheet this operation is called an SPI single-byte write.
*************************************************************************************/
static void write_RTC(unsigned char reg_RTC, unsigned char data_RTC) {
  SETBIT(PORTA, 1);             // Select DS1306

  /*----- Delay tcc -----*/
     __delay_cycles(16);
  /*---------------------*/
     
  /*----------------------- SEND ADDRESS -----------------------*/
  SPDR = reg_RTC;               // Send Address

  // --- Wait for Address to be sent --- //
  while(!(SPSR & (1 << SPIF))) {}           

  // --- Clear SPIF in SPSR --- //
  TESTBIT(SPSR, SPIF);
  TESTBIT(SPDR, 0);
 
  /*----------------------- SEND DATA -----------------------*/
  SPDR = data_RTC;              // Send Data
  
  // --- Wait for Data to be sent --- //
  while(!(SPSR & (1 << SPIF))) {} 

  // --- Clear SPIF in SPSR --- //
  TESTBIT(SPSR, SPIF);
  TESTBIT(SPDR, 0);
  
  /*----- Delay tcch -----*/
     __delay_cycles(1);
  /*---------------------*/
  
  CLEARBIT(PORTA, 1);           // De-select DS1306

  /*----- Delay tcwh -----*/
     __delay_cycles(16);
  /*---------------------*/
}

/*********************************************************************
 Function             : unsigned char read_RTC (unsigned char reg_RTC) 
 Date                 : 04/09/2018
 Version              : 1.0
 Target MCU           : ATmega128
 Author               : Wilmer Suarez
 DESCRIPTION
 This function reads data from a register in the RTC. 
 To accomplish this, it must first write the register's address 
 (reg_RTC) followed by writing a dummy byte to generate the SCLKs 
 to read the data (data_RTC). 
 In the DS1306 data sheet this operaration is called an SPI 
 single-byte read.
*********************************************************************/
unsigned char read_RTC(unsigned char reg_RTC) {
  SETBIT(PORTA, 1);             // Select DS1306

  /*----- Delay tcc -----*/
     __delay_cycles(16);
  /*---------------------*/

  /*----------------------- SEND ADDRESS -----------------------*/
  SPDR = reg_RTC;               // Send Address

  // --- Wait for Address to be sent --- //
  while(!(SPSR & (1 << SPIF))) {} 

  // --- Clear SPIF in SPSR --- //
  TESTBIT(SPSR, SPIF);
  TESTBIT(SPDR, 0);
  
 /*----------------------- READ DATA -----------------------*/
  SPDR = 0xFF;                  // Dummy data to start clock
  
  // --- Wait for Data to be read --- //
  while(!(SPSR & (1 << SPIF))) {}  

  data = SPDR;                  // Read data from SPDR buffer
  
  // --- Clear SPIF in SPSR --- //
  TESTBIT(SPSR, SPIF);
  TESTBIT(SPDR, 0);
  
  /*----- Delay tcch -----*/
     __delay_cycles(1);
  /*---------------------*/
  
  CLEARBIT(PORTA, 1);           // De-select DS1306

  /*----- Delay tcwh -----*/
     __delay_cycles(16);
  /*---------------------*/
     
  return data;
}

/*******************************************************************************
 Function Name        : void block_write_RTC (volatile unsigned char *array_ptr,
                        unsigned char strt_addr, unsigned char count) 
 Date                 : 04/09/2018
 Version              : 1.0
 Target MCU           : ATmega128
 Author               : Wilmer Suarez
 DESCRIPTION
 This function writes a block of data from an array to the DS1306. 
 strt_addr is the starting address to be written in the DS1306. 
 count is the number of data bytes to be transferred and array_ptr is 
 the address of the source array.
*******************************************************************************/
void block_write_RTC(volatile unsigned char *array_ptr, unsigned char strt_addr, unsigned char count) {
  SETBIT(PORTA, 1);             // Select DS1306

  /*----- Delay tcc -----*/
     __delay_cycles(16);
  /*---------------------*/
     
  /*----------------------- SEND ADDRESS -----------------------*/
  SPDR = strt_addr;             // Send Address
  
  // --- Wait for Address to be sent --- //
  while(!(SPSR & (1 << SPIF))) {}           

  // --- Clear SPIF in SPSR --- //
  TESTBIT(SPSR, SPIF);
  TESTBIT(SPDR, 0);
  
  /*----------------------- SEND DATA -----------------------*/
  for(int i = 0; i < count; i++) {
    SPDR = *(array_ptr + i);    // Send next byte of Data in array
    
    // --- Wait for Data to be sent --- //
    while(!(SPSR & (1 << SPIF))) {} 

    // --- Clear SPIF in SPSR --- //
    TESTBIT(SPSR, SPIF);
    TESTBIT(SPDR, 0);
  }

  /*----- Delay tcch -----*/
     __delay_cycles(1);
  /*---------------------*/
  
  CLEARBIT(PORTA, 1);           // De-select DS1306

  /*----- Delay tcwh -----*/
     __delay_cycles(16);
  /*---------------------*/
}

/*******************************************************************************
 Function Name        : void block_read_RTC (volatile unsigned char *array_ptr,
                        unsigned char strt_addr, unsigned char count)
 Date                 : 04/09/2018
 Version              : 1.0
 Target MCU           : ATmega128
 Author               : Wilmer Suarez
 DESCRIPTION
 This function reads a block of data from the DS1306 and transfers it to an
 array. 
 strt_addr is the starting address in the DS1306. 
 count is the number of data bytes to be transferred and array_ptr is
 the address of the destination array.
*******************************************************************************/
static void block_read_RTC(volatile unsigned char *array_ptr, unsigned char strt_addr, unsigned char count) {
  SETBIT(PORTA, 1);             // Select DS1306

  /*----- Delay tcc -----*/
     __delay_cycles(16);
  /*---------------------*/
     
  /*----------------------- SEND ADDRESS -----------------------*/
  SPDR = strt_addr;             // Send Address
  
  // --- Wait for Address to be sent --- //
  while(!(SPSR & (1 << SPIF))) {}           

  // --- Clear SPIF in SPSR --- //
  TESTBIT(SPSR, SPIF);
  TESTBIT(SPDR, 0);
  
  /*----------------------- READ DATA -----------------------*/
  for(int i = 0; i < count; i++) {
    SPDR = 0xFF;                // Dummy data to start clock

    // --- Wait for Data to be read --- //
    while(!(SPSR & (1 << SPIF))) {} 

    array_ptr[i] = SPDR;        // Read next byte of data from SPDR buffer

    // --- Clear SPIF in SPSR --- //
    TESTBIT(SPSR, SPIF);
    TESTBIT(SPDR, 0);
    
  }

  /*----- Delay tcch -----*/
     __delay_cycles(1);
  /*---------------------*/
  
  CLEARBIT(PORTA, 1);           // De-select DS1306

  /*----- Delay tcwh -----*/
     __delay_cycles(16);
  /*---------------------*/
}