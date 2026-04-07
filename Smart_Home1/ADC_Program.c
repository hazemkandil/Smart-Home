/*
 * ADC_Program.c  –  Smart_Home1 (ATmega32 – keypad MCU)
 *
 *  Created on: Sep 6, 2022
 *      Author: Hazem Mohamed
 *
 *  Description:
 *      ADC driver for the keypad MCU.  Smart_Home1 does not read any
 *      analog sensors itself; this driver is included for completeness
 *      and potential future use (e.g., reading a potentiometer for
 *      servo position feedback).
 *
 *      Configuration:
 *        Reference voltage : AREF (external, 5 V)
 *        Left adjustment   : OFF  → right-aligned 10-bit result
 *        Auto-trigger      : ON   → free-running mode
 *        Prescaler         : DIV_64 → ADC clock = 8 MHz / 64 = 125 kHz
 */

#include "Bit_Math.h"
#include "Std_Types.h"

#include "ADC_Interface.h"
#include "ADC_Registers.h"

/* Initialise the ADC peripheral. */
void ADC_voidInit(void) {
	/* Voltage reference: AREF (REFS1:REFS0 = 00) */
	ADMUX = AREF << 6;

	/* Right-align result (ADLAR = 0) */
	ADMUX |= ADCLEFT_ADJUSTMENT_OFF << 5;

	/* Enable Auto-Trigger (free-running mode) */
	Set_Bit(ADCSRA, ADCSRA_ADATE);

	/* Enable the ADC module */
	Set_Bit(ADCSRA, ADCSRA_ADEN);

	/* Prescaler = DIV_64 → ADC clock = 125 kHz */
	ADCSRA |= DIV_64 << 0;

	/* Clear the ADC Interrupt Flag to start clean */
	Set_Bit(ADCSRA, ADCSRA_ADIF);
}

/* Select channel, trigger a single conversion, wait for completion,
 * and return the 10-bit result (right-aligned in ADCLH). */
u16 ADC_u16Read(ADC_CHANNELS_options_t Copy_u8ADCChannel) {
	/* Clear channel selection bits then set the requested channel */
	ADMUX |= Copy_u8ADCChannel << 0;

	/* Start conversion */
	Set_Bit(ADCSRA, ADCSRA_ADSC);

	/* Wait for ADIF (conversion complete) */
	while (!Get_Bit(ADCSRA, ADCSRA_ADIF))
		;

	/* Clear ADIF for next conversion */
	Set_Bit(ADCSRA, ADCSRA_ADIF);

	return ADCLH;
}

/* Convert raw ADC value to voltage: V = (ADC_raw / 1024) × VREF */
f32 ADC_f32ADCtoVolt(u16 Copy_u16ADCVal) {
	f32 ADC_Localf32Volt = (Copy_u16ADCVal / 1024.0) * VREF;
	return ADC_Localf32Volt;
}
