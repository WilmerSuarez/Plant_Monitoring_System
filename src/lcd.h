/**********************************************************************

 File Name            : lcd.h
 Title                : Header file for LCD module
 Date                 : 02/07/10
 Version              : 1.0
 Target MCU           : ATmega128 @  MHz
 Target Hardware      ; 
 Author               : Ken Short
 DESCRIPTION
 This file includes all the declaration the compiler needs to 
 reference the functions and variables written in the files lcd_ext.c.
 lcd.h and lcd_dog_iar_driver.c
**********************************************************************/

/**
 *  This declaration tells the compiler to look for dsp_buff_x in
 *  another module. It is used by lcd_ext.c and main.c to locate the buffers.
 */
extern char dsp_buff_1[16];
extern char dsp_buff_2[16];
extern char dsp_buff_3[16];

/**
 *  Declaratios of low level lcd functions located in lcd_dog_iar_driver.c
 *  Note that these are external.
 */
extern void init_lcd_dog();
extern void update_lcd_dog();

/**
 *  These functions are located in lcd_ext.c
 */
extern int putchar(int);
