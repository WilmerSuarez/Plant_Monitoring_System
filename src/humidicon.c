/*************************************************
 File Name              : "humidicon.c" 
 Title                : Humidicon Test Functions
 Date                 : 04/09/2018  
 Version              : 1.0 
 Target MCU           : ATMEGA128A
 Author               : Wilmer Suarez 
 DESCRIPTION
 This file contains the functions needed to get 
 the temperature and humidity from the humidicon.
 The values are then printed on the LCD. 
*************************************************/

// ----- Include Files ----- //
#include "header.h"     // Includes the ATmega128 Definitions, Macros, and Instinsic functions
#include "lcd.h"
#include <stdio.h>

// ---------- Global static Variables ---------- //
static unsigned int humidicon_byte1;        // First byte of Humidicon data
static unsigned int humidicon_byte2;        // Second byte of Humidicon data
static unsigned int humidicon_byte3;        // Third byte of Humidicon data
static unsigned int humidicon_byte4;        // Fourth byte of Humidicon data
static unsigned int humidity_raw;           // Raw data for humidity 
static unsigned int temperature_raw;        // Raw data for temperature
static unsigned int humidity;               // Computed scaled Humidity
static unsigned int temperatureC;           // Computed scaled Temperature in Celcius
static unsigned int temperatureF;           // Temperature in Fahrenheit

// ---------- Static Function Prototypes ---------- //
static void SPI_humidicon_config();
static unsigned char read_humidicon_byte();
static void read_humidicon();
static unsigned int compute_scaled_rh(unsigned int rh);
static unsigned int compute_scaled_temp(unsigned int temp);

/***********************************************************************
 Function             : void meas_display_rh_temp()
 Date                 : 04/09/2018
 Version              : 1.0
 Target MCU           : ATmega128
 Author               : Wilmer Suarez
 DESCRIPTION
 This function configures the Humidicon for SPI and gets the calculated
 temperature and humidity values from the Humidicon. The function then
 displays the results on a DOG 3 x 16 LCD in the following format:
 Time: hh:mm:ss - other file
 Temp: temp°C
 RH:   rh%
***********************************************************************/
void meas_display_rh_temp() {
  // ---------- Configure MCU for Humidicon SPI ---------- //
  SPI_humidicon_config();
  
  // --------- Get Scaled temperature and Humidity values ---------- //
  read_humidicon();
  
  // ------------ Print Temperature and Humidity ------------ //
  if(tempCF == true) {  // Display temperature in degrees Celcius
    printf("Temp: %d.%d%cC\nRH:   %d.%d%%", 
           (temperatureC / 100), (temperatureC % 100), 0xDF, (humidity / 100), (humidity % 100));
  } else {      // Display temperature in degrees Fahrenheit
    float c = 9/5;
    temperatureF = (int) ((temperatureC / 100) * c) + 32;
    printf("Temp: %d.%d%cF\nRH:   %d.%d%%", 
           temperatureF, (temperatureF % 100), 0xDF, (humidity / 100), (humidity % 100));
  }
  
  init_lcd_dog();                   // Initialize the Display
  update_lcd_dog();                 // Updates the LCD to display the current time, temperature, and humidity stored in the display buffers
}

/***************************************************
 Function             : void SPI_humidicon_config()
 Date                 : 04/09/2018
 Version              : 1.0
 Target MCU           : ATmega128
 Author               : Wilmer Suarez
 DESCRIPTION
 This function unselects the HumidIcon and 
 configures it for operation with
 an ATmega128A operated a 16 MHz. Pin PA0 of the 
 ATmega128A is used to select the HumidIcon
***************************************************/
void SPI_humidicon_config() {
  // ---------------- PORT Configuration ---------------- //
  SETBIT(PORTA, 0); // Initially de-select HumidIcon
  
  // ---------------- SPI Configuration ---------------- //
    
  // Set SPE, MSTR, and SPR1 (fosc/64) //
  SPCR = (1 << SPE) | (1 << MSTR) | (1 << SPR1); 

  // Clear SPIF in SPSR //
  TESTBIT(SPSR, SPIF);
  TESTBIT(SPDR, 0);
}

/**************************************************************
 Function             : void read_humidicon()
 Date                 : 04/09/2018
 Version              : 1.0
 Target MCU           : ATmega128
 Author               : Wilmer Suarez
 DESCRIPTION
 This function selects the Humidicon by asserting PA0. 
 It then calls read_humidicon_byte() four times to read 
 the temperature and humidity information. Is assigns 
 the values read to the global unsigned ints 
 humidicon_byte1, humidion_byte2, humidion_byte3, and 
 humidion_byte4, respectively. The function then 
 deselects the HumidIcon. 
 
 The function then extracts the fourteen bits 
 corresponding to the humidity information and stores 
 them right justified in the global unsigned int humidity_raw.
 Next it extracts the fourteen bits corresponding to 
 the temperature information and stores them in the global 
 unsigned int temperature_raw. The function then returns
**************************************************************/
void read_humidicon() {
  // Select Humidicon as Slave //
  CLEARBIT(PORTA, 0);
    
  SPDR = 0xFF;  // Measurement request Command
  
  // Wait for measuremnt cycle to complete (36.65 ms) // 
  __delay_cycles(16 * 36650);
   
  // --------------- Read the 4 bytes of valid data from the Humidicon --------------- // 
  // Read the first byte of Humidicon Data //  
  humidicon_byte1 = read_humidicon_byte();
  
  humidicon_byte1 &= 0x3F;      // Mask first two bits of data (status bits) 
  
  // Read the second byte of Humidicon Data //
  humidicon_byte2 = read_humidicon_byte(); 
    
  // Read the third byte of Humidicon Data //
  humidicon_byte3 = read_humidicon_byte(); 
  
  // Read the fourth byte of Humidicon Data //
  humidicon_byte4 = read_humidicon_byte(); 
  
  // De-select Humidicon as Slave //
  SETBIT(PORTA, 0);
  
  // ----- Get 14 bits of Humidity and 14 bits of Temperature and store ----- //
  // ----- them in respective Varaibles ----- //
  humidity_raw = (humidicon_byte1 << 8) | (humidicon_byte2);   
  temperature_raw = (humidicon_byte3 << 6) | (humidicon_byte4 >> 2);
  
  // ---------- Compute scaled value of Humidity and Temperature ---------- //
  humidity = compute_scaled_rh(humidity_raw);
  temperatureC = compute_scaled_temp(temperature_raw);  
}

/****************************************************************
 Function             : unsigned char read_humidicon_byte()
 Date                 : 04/09/2018
 Version              : 1.0
 Target MCU           : ATmega128
 Author               : Wilmer Suarez
 DESCRIPTION
 This function reads a data byte from the HumidIcon sensor and 
 returns it as an unsigned char. The function does not return 
 until the SPI transfer is completed. The function determines 
 whether the SPI transfer is complete by polling the appropriate 
 SPI status flag.
****************************************************************/
unsigned char read_humidicon_byte() {
  // Write dummy data to SPDR (To initiate Clock) //
  SPDR = 0xFF; 
  
  // Wait until humidicon sends first byte of data // 
  while(!(SPSR & (1 << SPIF)));
  
  // Read byte from Humidicon // 
  unsigned char dataByte = SPDR;   
  
  // Clear SPIF //
  TESTBIT(SPSR, SPIF);
  TESTBIT(SPDR, 0);

  // Return the byte of data read from the Humidicon // 
  return dataByte;
}

/****************************************************************************
 Function             : unsigned int compute_scaled_rh(unsigned int rh)
 Date                 : 04/09/2018
 Version              : 1.0
 Target MCU           : ATmega128
 Author               : Wilmer Suarez
 DESCRIPTION
 Computess scaled relative humidity in units of 0.01% RH from the raw 14-bit
 realtive humidity value from the Humidicon.
****************************************************************************/
unsigned int compute_scaled_rh(unsigned int rh) {
  // --------------- Convert humidity raw data --------------- // 
  unsigned int hum = ((unsigned long)rh * 10000) / (16380); // Scaling of 0.01% RH
  return hum;
}

/****************************************************************************
 Function             : unsigned int compute_scaled_temp(unsigned int temp)
 Date                 : 04/09/2018
 Version              : 1.0
 Target MCU           : ATmega128
 Author               : Wilmer Suarez
 DESCRIPTION
 Computess scaled temperature in units of 0.01 degrees C from the raw 14-bit
 temperature value from the Humidicon
****************************************************************************/
unsigned int compute_scaled_temp(unsigned int temp) {
  // --------------- Convert temperature raw data --------------- // 
  unsigned int t = (((unsigned long)temp * 16500) / (16380)) - 4000;  // Scaling of 0.01°C
  return t;
}