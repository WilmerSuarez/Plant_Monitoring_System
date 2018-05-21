/****************************************************************
 File Name            : "fsm_functions.c" 
 Title                : FSM Functions
 Date                 : 04/09/2018  
 Version              : 1.0 
 Target MCU           : ATMEGA128A
 Author               : Wilmer Suarez 
 DESCRIPTION 
 This file includes all the functions associated with the states
 of the Table Driven FSM syste. 
****************************************************************/   

// ----- Include Files ----- //
#include "header.h"             // Includes the ATmega128 Definitions, Macros, and Instinsic functions
#include <stdio.h>
#include "DS1306.h"
#include "FSM.h"                // FSM State Function declerations
#include "lcd.h"
int result;            // Holds the result of the ADC conversion for CO2 measurements
long decimalnum, quotient, remainder;   // Used when converting int to Hex
char hex[3];                    // Holds the Hex values
static int positionT = 0;       // Keeps track of the LCD position for changeTime_fn
static int positionA = 0;       // Keeps track of the LCD position for changeAlarm0_fin
static int time = 0;            // timeValues array index
static int indexM = 0;          // monthVal array index
static int indexD = 0;          // dateVal array index
static int indexY = 0;          // yearVal array index
unsigned char RTC_write_time[7];// Holds the values to be written to the DS1306 registers
unsigned char RTC_write_alarm[4];
unsigned char timeValues[6];    // Holds the time
unsigned char dayVal;           // Holds the day of the week
unsigned char dateVal[2];       // Holds the day of the month      
unsigned char monthVal[2];      // Holds the month
unsigned char yearVal[2];       // Holds the year
unsigned char alarmVal;         // Holds the type of alarm
unsigned char *aPtr;  
unsigned char hours, minutes, seconds, day, month, year;

/******************************************************
 Function             : void changeTime_fn(key keyVal)
 Date                 : 04/09/2018
 Version              : 1.0
 Target MCU           : ATmega128
 Author               : Wilmer Suarez
 DESCRIPTION
 Receives input form the keypad to update the value of 
 the time registers and the date registers
 (Hours, Minutes, Seconds, ect...) of the DS1306.
******************************************************/
void changeTime_fn(key keyVal) {
  // --- INPUT MONTH --- //
  if(positionT == 0) {
    printf("\f  Enter Month:\nJan->Dec (1->12)       mm\b\b");
    positionT++;
  } else if(positionT <= 2){
    monthVal[indexM++] = keyVal;
    printf("%d", keyVal);
    positionT++;
    init_lcd_dog();                   // Initialize the Display
    update_lcd_dog();                 // Updates the LCD to display the key value entered by the user
    if(positionT == 3) {
      __delay_cycles(16000000);        // Delay for 1 seconds
      printf("\f   Enter Day:\n     01->31\n       dd\b\b");
      positionT++; 
    }  
  // --- INPUT DAY OF THE MONTH --- //
  } else if(positionT <= 5) {
    dateVal[indexD++] = keyVal;
    printf("%d", keyVal);                
    positionT++;
    init_lcd_dog();                   // Initialize the Display
    update_lcd_dog();                 // Updates the LCD to display the key value entered by the user
    if(positionT == 6) {
      __delay_cycles(16000000);         // Delay for 1 seconds
      printf("\f   Enter Year:\n     00->99\n       YY\b\b");
      positionT++; 
    }
  // --- INPUT YEAR --- //
  } else if(positionT <= 8) {
    yearVal[indexY++] = keyVal;
    printf("%d", keyVal);               
    positionT++;
    init_lcd_dog();                   // Initialize the Display
    update_lcd_dog();                 // Updates the LCD to display the key value entered by the user
    if(positionT == 9) {
      __delay_cycles(16000000);         // Delay for 1 seconds
       printf("\f Enter Weekday:\nMon->Sun (1->7)        d\b");
       positionT++; 
    }
  // --- INPUT DAY OF WEEK --- // 
  } else if(positionT == 10){
    dayVal = keyVal;
    printf("%d", keyVal);
    init_lcd_dog();                   // Initialize the Display
    update_lcd_dog();                 // Updates the LCD to display the key value entered by the user
    positionT++;
    __delay_cycles(16000000);         // Delay for 2 seconds        
  // --- INPUT TIME --- // 
    printf("\fChange the Time:    HH:mm:ss\b\b\b\b\b\b\b\b"); 
  } else {
    if(positionT <= 18) {
      if(positionT == 13 || positionT == 16) {       // Skip the colons
        printf(":");
        positionT++;
      }
      timeValues[time++] = keyVal;              
      printf("%d", keyVal);
      positionT++;
      init_lcd_dog();                   // Initialize the Display
      update_lcd_dog();                 // Updates the LCD to display the key value entered by the user
    } 
    if(positionT == 19) {
      __delay_cycles(16000000);               
      // Setup the time and day registers in the format required for the DS1306
      hours = (timeValues[0] << 4) | timeValues[1];
      minutes = (timeValues[2] << 4) | timeValues[3];
      seconds = (timeValues[4] << 4) | timeValues[5];
      day = (dateVal[0] << 4) | dateVal[1];
      month = (monthVal[0] << 4) | monthVal[1];
      year = (yearVal[0] << 4) | yearVal[1];
      RTC_write_time[0] = seconds;
      RTC_write_time[1] = minutes;
      RTC_write_time[2] = hours;
      RTC_write_time[3] = dayVal;
      RTC_write_time[4] = day;
      RTC_write_time[5] = month;
      RTC_write_time[6] = year;
      
      if((hours <= 0x23) && (minutes <= 0x59) && (seconds <= 0x59) && (dayVal <= 0x07)
         && (day <= 0x31) && (month <= 0x12) && (year <= 0x99)) {
        // Configure Microcontroller SPI to communicate with the DS1306 RTC
        SPI_rtc_DS1306_config();
        aPtr = RTC_write_time;                 // Pointing to start of write Array
        block_write_RTC(aPtr, 0x80, 7);
        printf("\f");
        positionT = 0;                     // Reset start position
        time = 0;                         // Reset timeValues array start index
        indexM = 0;                       // Reset index
        indexD = 0;                       // Reset index
        indexY = 0;                       // Reset index
        present_state = idle;
      } else {
        positionT = 0;                     
        time = 0;                         
        indexM = 0;                       
        indexD = 0;                    
        indexY = 0;                       
        printf("\f  Invalid Time\n       or\n  Invalid Date");
        init_lcd_dog();                   // Initialize the Display
        update_lcd_dog();                 // Updates the LCD to display the key value entered by the user
        __delay_cycles(32000000);
        present_state = idle;
      }
    }
  }
  init_lcd_dog();                   // Initialize the Display
  update_lcd_dog();                 // Updates the LCD to display the key value entered by the user
} 

/********************************************************
 Function             : void changeAlarm0_fn(key keyVal)
 Date                 : 04/09/2018
 Version              : 1.0
 Target MCU           : ATmega128
 Author               : Wilmer Suarez
 DESCRIPTION
 Receives input form the keypad to update the value of 
 the Alarm0 registers (Hours, Minutes, Seconds, and Day) 
 of the DS1306.
********************************************************/
void changeAlarm0_fn(key keyVal) {
  // --- INPUT ALARM TYPE --- //
  if(positionA == 0) {
    printf("\f Choose alrarm:\n     1->5:\n       a\b");
    positionA++;
  } else if(positionA == 1) {
    alarmVal = keyVal;
    printf("%d", keyVal);
    init_lcd_dog();                   // Initialize the Display
    update_lcd_dog();                 // Updates the LCD to display the key value entered by the user
    positionA++;
    __delay_cycles(16000000);                    // Delay for 2 seconds
   // --- INPUT DAY OF THE WEEK --- //
    printf("\f Enter Weekday:\nMon->Sun (1->7)        d\b");
  } else if(positionA == 2){
    dayVal = keyVal;
    printf("%d", keyVal);
    init_lcd_dog();                   // Initialize the Display
    update_lcd_dog();                 // Updates the LCD to display the key value entered by the user
    positionA++;
    __delay_cycles(16000000);  
   // --- INPUT ALARM TIME --- //
    printf("\f Change Alarm0:\n    HH:mm:ss\b\b\b\b\b\b\b\b");  
  } else {
    if(positionA <= 10) {
      if(positionA == 5 || positionA == 8) {       // Skip the colons
        printf(":");
        positionA++;
      }
      timeValues[time++] = keyVal;               // Update the array holding the input key values
      printf("%d", keyVal);
      positionA++;
      init_lcd_dog();                   // Initialize the Display
      update_lcd_dog();                 // Updates the LCD to display the key value entered by the user
    } 
    if(positionA == 11) {
      __delay_cycles(16000000);                  
      // Setup the Alarm0 values in the format required for the DS1306
      switch(alarmVal) {
        case 1: // Alarm every second
          hours = ((timeValues[0] << 4) | timeValues[1]) | 0x80;    
          minutes = ((timeValues[2] << 4) | timeValues[3]) | 0x80;
          seconds = ((timeValues[4] << 4) | timeValues[5]) | 0x80;
          dayVal = dayVal | 0x80;
          RTC_write_alarm[0] = seconds;
          RTC_write_alarm[1] = minutes;
          RTC_write_alarm[2] = hours;
          RTC_write_alarm[3] = dayVal;
          break;
        case 2: // Alarm every minute
          hours = ((timeValues[0] << 4) | timeValues[1]) | 0x80;    
          minutes = ((timeValues[2] << 4) | timeValues[3]) | 0x80;
          seconds = (timeValues[4] << 4) | timeValues[5];
          dayVal = dayVal | 0x80;
          RTC_write_alarm[0] = seconds;
          RTC_write_alarm[1] = minutes;
          RTC_write_alarm[2] = hours;
          RTC_write_alarm[3] = dayVal;
          break;
        case 3: // Alarm every hour
          hours = ((timeValues[0] << 4) | timeValues[1]) | 0x80;    
          minutes = (timeValues[2] << 4) | timeValues[3];
          seconds = (timeValues[4] << 4) | timeValues[5];
          dayVal = dayVal | 0x80;
          RTC_write_alarm[0] = seconds;
          RTC_write_alarm[1] = minutes;
          RTC_write_alarm[2] = hours;
          RTC_write_alarm[3] = dayVal;
          break;
        case 4: // Alarm every day
          hours = (timeValues[0] << 4) | timeValues[1];    
          minutes = (timeValues[2] << 4) | timeValues[3];
          seconds = (timeValues[4] << 4) | timeValues[5];
          dayVal = dayVal | 0x80;
          RTC_write_alarm[0] = seconds;
          RTC_write_alarm[1] = minutes;
          RTC_write_alarm[2] = hours;
          RTC_write_alarm[3] = dayVal;
          break;
        case 5: // Alarm every week
          hours = (timeValues[0] << 4) | timeValues[1];    
          minutes = (timeValues[2] << 4) | timeValues[3];
          seconds = (timeValues[4] << 4) | timeValues[5];
          dayVal = dayVal;
          RTC_write_alarm[0] = seconds;
          RTC_write_alarm[1] = minutes;
          RTC_write_alarm[2] = hours;
          RTC_write_alarm[3] = dayVal;
          break;
      }
      
      // Configure Microcontroller SPI to communicate with the DS1306 RTC
      SPI_rtc_DS1306_config();
      aPtr = RTC_write_alarm;                 // Pointing to start of write Array
      block_write_RTC(aPtr, 0x87, 4);
      printf("\f");
      positionA = 0;                     // Reset start position
      time = 0;                         // Reset timeValues array start index
      present_state = idle;

    }
  }
  init_lcd_dog();                   // Initialize the Display
  update_lcd_dog();                 // Updates the LCD to display the key value entered by the user
}

/****************************************************
 Function             : void idle_fn(key keyVal)
 Date                 : 04/11/2018
 Version              : 1.0
 Target MCU           : ATmega128
 Author               : Wilmer Suarez
 DESCRIPTION
 This function is called when a user entered the 
 changeTime or changeAlarm0 state, accidentily, and 
 wish to return back to the idle state.
****************************************************/
void idle_fn(key keyVal) {
  // Do nothing
}

/******************************************************
 Function             : extern void back_fn(key keyVal)
 Date                 : 04/11/2018
 Version              : 1.0
 Target MCU           : ATmega128
 Author               : Wilmer Suarez
 DESCRIPTION
 This function is called when an accidental number is 
 pressed. It moves the cursor position back 1 position.
******************************************************/
extern void back_fn(key keyVal) {
  // --- For changeTime function --- //
  if(positionT == 2) {
    indexM--;
    positionT--;
    printf("\b_\b");
  }
  if(positionT == 5) {
    indexD--;
    positionT--;
    printf("\b_\b");
  }
  if(positionT == 8) {
    indexY--;
    positionT--;
    printf("\b_\b");
  }
  if(positionT == 14 || positionT == 17) {
    time--;
    positionT -= 2;
    printf("\b\b_\b");
  }
  if(positionT == 12 || positionT == 15 || positionT == 18) {
    time--;
    positionT--;
    printf("\b_\b");
  }
  
  // --- For changeAlarm0 function --- //
  if(positionA == 4 || positionA == 7 || positionA == 10) {
    time--;
    positionA--;
    printf("\b_\b");
  }
  if(positionA == 6 || positionA == 9) {
    time--;
    positionA -= 2;
    printf("\b\b_\b");
  }
}

/****************************************************
 Function             : void dispCO2_fn(key keyVal)
 Date                 : 04/22/2018
 Version              : 1.0
 Target MCU           : ATmega128 @ 16MHz
 Author               : Wilmer Suarez
 DESCRIPTION
 Displays the CO2 measurement.
****************************************************/
void dispCO2_fn(key keyVal) {
  // Start ADC Conversion
  SETBIT(ADCSRA, ADSC);

  // Wait for ADC converion is complete
  while(!(ADCSRA & (1 << ADIF)));

  // Clearn  ADC Interrupt Flag
  SETBIT(ADCSRA, ADIF);

  // Display the Conversion
  display();
}

/****************************************************
 Function             : void display()
 Date                 : 04/22/2018
 Version              : 1.0
 Target MCU           : ATmega128 @ 16MHz
 Author               : Wilmer Suarez
 DESCRIPTION
 Reads and converts the ADC result to Hex.
 The result is printed to the LCD.
****************************************************/
void display() {
  result = ADCL;                // Get Low 8-bits of ADC Result
  result |= (ADCH << 8);        // Get High 2-bits of ADC Result 
  float voltage = result * (2560/1024.0);
  if(voltage == 0) {
    printf("\f      CO2:\n");    
    printf("      Fault");
  } else if(voltage < 400) {
    printf("\f      CO2:\n");    
    printf("   Preheating");
  } else {
    int voltage_difference = (int) voltage - 400;
    float concentration = voltage_difference * (50.0/16.0);
    printf("\fV: %.2fmv\n", voltage);   
    printf("CO2: %.2fppm\n", concentration);    
  }
  init_lcd_dog();               // Initialize the LCD
  update_lcd_dog();             // Update the LCD with the contents of the display buffers
}

/****************************************************
 Function             : void error_fn(key keyVal)
 Date                 : 04/09/2018
 Version              : 1.0
 Target MCU           : ATmega128 @ 16MHz
 Author               : Wilmer Suarez
 DESCRIPTION
 Temporarily Displays an error message on the third 
 line of the LCD screen when an incorrect key is 
 pressed.
****************************************************/
void error_fn(key keyVal) {
  if(present_state == idle) {
    printf("\f Invalid Input!");
    init_lcd_dog();                     // Initialize the Display
    update_lcd_dog();                   // Updates the LCD to display the error message
    __delay_cycles(32000000);           // Delay for 2 seconds
  } else {
    
  }
}
