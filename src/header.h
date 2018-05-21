/************************************************************
  File Name            : "header.h" 
  Title                : Program Header File
  Date                 : 04/09/2018  
  Version              : 2.0  
  Target MCU           : ATMEGA128A
  Author               : Wilmer Suarez 
  DESCRIPTION 
  This header file includes all the directives needed by the 
  program. It also declares the 'boolean' variable that 
  determines if the displayed temperature is in celcius or
  fahrenheit.
************************************************************/
typedef int bool;
#define true 1
#define false 0

extern bool tempCF;    // True if Celcius, False if Fahrenheit

// ----- Include Files ----- //
#include <iom128.h>             // Includes the ATmega128 Definitions
#include <intrinsics.h>         // Includes helpful Macros
#include <avr_macros.h>         // Includes the ATmega128 Instinsic functions
