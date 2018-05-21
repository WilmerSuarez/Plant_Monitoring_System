/****************************************************************
  File Name            : "DS1306_RTC.h" 
  Title                : DS1306 Header File
  Date                 : 04/09/2018  
  Version              : 1.0 
  Target MCU           : ATMEGA128A
  Author               : Wilmer Suarez 
  DESCRIPTION 
  This header file includes the external 
  function declerations used for the DS1306.
****************************************************************/ 

// ------- External Functions for the DS1306 ------- //
extern void DS1306_RTC_config();
extern void SPI_rtc_DS1306_config();
extern unsigned char read_RTC(unsigned char reg_RTC);
extern void block_write_RTC(volatile unsigned char *array_ptr, unsigned char start_addr, unsigned char count);