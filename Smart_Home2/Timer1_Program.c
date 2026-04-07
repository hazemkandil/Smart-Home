/*
 * Timer1_Program.c
 *
 *  Created on: Sep 8, 2022
 *      Author: Hazem Mohamed
 *
 *  Description:
 *      Timer1 PWM driver for the ATmega16 (Smart_Home2 – display/control MCU).
 *      Configures Timer1 in Fast PWM Mode 14 (ICR1 as TOP) to generate
 *      a 50 Hz PWM signal on OC1A, which drives the door servo motor.
 *
 *      Servo control:
 *        - ICR1 = 20000 ticks → period = 20 ms  (50 Hz) @ prescaler div/8, 8 MHz
 *        - OCR1A = 750  → pulse ≈ 0.75 ms → servo at ~0° (door closed)
 *        - OCR1A = 1250 → pulse ≈ 1.25 ms → servo at ~90° (door open)
 *      The door-open sweep in main.c increments OCR1A from 751 to 1250
 *      one tick at a time with a 1 ms delay for a smooth rotation.
 */

#include "Std_Types.h"
#include "Bit_Math.h"

#include "Timer1_Registers.h"
#include "Timer1_Interface.h"

/* Initialise Timer1 for Fast PWM Mode 14 on OC1A (servo motor output). */
void PWM_voidInitTimer1()
{
	/* Non-inverting PWM on OC1A:
	 *   COM1A1=1, COM1A0=0 → clear OC1A on compare match, set at BOTTOM */
	Clear_Bit(TCCR1A, TCCR1A_COM1A0);
	Set_Bit  (TCCR1A, TCCR1A_COM1A1);

	/* Waveform Generation Mode 14 (Fast PWM, TOP = ICR1):
	 *   WGM13=1, WGM12=1, WGM11=1, WGM10=0 */
	Clear_Bit(TCCR1A, TCCR1A_WGM10);
	Set_Bit  (TCCR1A, TCCR1A_WGM11);
	Set_Bit  (TCC1RB, TCC1RB_WGM12);
	Set_Bit  (TCC1RB, TCC1RB_WGM13);

	/* Clock prescaler = div/8:
	 *   CS12:CS11:CS10 = 010
	 *   f_timer = 8 MHz / 8 = 1 MHz → tick period = 1 µs */
	TCC1RB &= 0b11111000;
	TCC1RB |= CLK_DIV_8 << TCC1RB_CS10_CS11_CS12;

	/* TOP = 20000 ticks → PWM period = 20000 µs = 20 ms (50 Hz) */
	ICR1 = 20000;

	/* Initial compare value set dynamically via PWM_voidSetCompareMatchValue(). */
	//OCR1A = 750;
}

/* Update the PWM duty cycle (servo position) at runtime.
 * Call repeatedly with increasing values (751→1250) to sweep the door open. */
void PWM_voidSetCompareMatchValue(u16 Copy_u16value)
{
	/*Set the number of ticks of compare match (Compare Value)*/
	OCR1A = Copy_u16value;
}
