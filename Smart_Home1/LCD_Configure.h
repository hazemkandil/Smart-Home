/*
 ************ Author : Hazem Mohamed ****************
 ************ Date : 31-8-2022 *******************
 *************************************************
 */

#ifndef LCD_CONFIGURE_H_
#define LCD_CONFIGURE_H_

// PINS LCD Control

#define LCD_CTRL_PORT   DIO_PORTD

#define LCD_RS_PIN      PIN2
#define LCD_RW_PIN      PIN3
#define LCD_E_PIN       PIN4

// PINS Data (Port)

#define LCD_DATA_PORT    DIO_PORTC


#define LCD_LINE_ONE     0
#define LCD_LINE_TWO     1

// patterns in CGRAAM

#define PATTERN_0      0
#define PATTERN_1      1
#define PATTERN_2      2
#define PATTERN_3      3
#define PATTERN_4      4
#define PATTERN_5      5
#define PATTERN_6      6
#define PATTERN_7      7
#endif /* LCD_CONFIGURE_H_ */


