/*
 * Timer1_Program.c
 *
 *  Created on: Sep 8, 2022
 *      Author: Hazem Mohamed
 *
 *  Description:
 *      Timer1 PWM driver for the ATmega32 (Smart_Home1 – keypad MCU).
 *      Configures Timer1 in Fast PWM Mode 14 (ICR1 as TOP) to drive
 *      a servo motor on OC1A at 50 Hz.
 *
 *      Servo timing (@ 8 MHz, prescaler div/8 → 1 µs/tick):
 *        ICR1  = 20000 → period = 20 ms  (50 Hz)
 *        OCR1A =   750 → pulse ≈ 0.75 ms → ~0°  (door closed)
 *        OCR1A =  1250 → pulse ≈ 1.25 ms → ~90° (door open)
 *      main.c sweeps OCR1A from 751 → 1250 with 1 ms delay per step
 *      for smooth door rotation.
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
	 *   CS12:CS11:CS10 = 010  → f_timer = 1 MHz → tick = 1 µs */
	TCC1RB &= 0b11111000;
	TCC1RB |= CLK_DIV_8 << TCC1RB_CS10_CS11_CS12;

	/* TOP = 20000 ticks → period = 20 ms (50 Hz servo signal) */
	ICR1 = 20000;

	/* Initial compare value set by main.c sweep; left commented here.
	 * Uncomment to force a fixed start position at door-closed (0°). */
	/* OCR1A = 750; */
}

/* Update the servo position by changing the PWM pulse width.
 * Sweep from 751 to 1250 (one tick per call with 1 ms delay) to
 * smoothly rotate the door from closed to open. */
void PWM_voidSetCompareMatchValue(u16 Copy_u16value)
{
	/*Set the number of ticks of compare match (Compare Value)*/
	OCR1A = Copy_u16value;
}
