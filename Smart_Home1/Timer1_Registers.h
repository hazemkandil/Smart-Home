/*
 * Timer1_Registers.h
 *
 *  Created on: Sep 8, 2022
 *      Author: Hazem Mohamed
 */

#ifndef TIMER1_REGISTERS_H_
#define TIMER1_REGISTERS_H_

#define TCCR1A        *((volatile u8*)0x4F)

#define TCCR1A_WGM10      0
#define TCCR1A_WGM11      1
#define TCCR1A_COM1A0     6
#define TCCR1A_COM1A1     7

#define TCC1RB       *((volatile u8*)0x4E)

#define TCC1RB_WGM12           3
#define TCC1RB_WGM13           4
#define TCC1RB_CS10_CS11_CS12  0

#define OCR1A        *((volatile u16*)0x4A)

#define ICR1         *((volatile u16*)0x46)

typedef enum
{
	No_ClocK,
	No_Prescaler,
	CLK_DIV_8,
	CLK_DIV_64,
	CLK_DIV_256,
	CLK_DIV_1024
}Prescaler_Options_t;

#endif /* TIMER1_REGISTERS_H_ */
