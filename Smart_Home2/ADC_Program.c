/*
 * ADC_Program.c  –  Smart_Home2 (ATmega16 – display/control MCU)
 *
 *  Created on: Sep 6, 2022
 *      Author: Hazem Mohamed
 *
 *  Description:
 *      ADC (Analog-to-Digital Converter) driver for reading sensor inputs.
 *      Used by Smart_Home2 to read two analog sensors:
 *        ADC0 (PORTA PIN0) – LDR light sensor   → controls 8 LED levels
 *        ADC1 (PORTA PIN1) – LM35 temperature   → controls fan on PORTA PIN3
 *
 *      Configuration:
 *        Reference voltage : AREF (external, 5 V)
 *        Left adjustment   : OFF  → result is right-aligned in ADCL/ADCH
 *        Auto-trigger      : ON   → continuous free-running conversion
 *        Prescaler         : DIV_64 → ADC clock = 8 MHz / 64 = 125 kHz
 *                            (within the recommended 50–200 kHz range)
 *
 *      Voltage formula:
 *        V = (ADC_raw / 1024.0) × VREF   (VREF = 5 V, defined in ADC_Interface.h)
 */

#include "Std_Types.h"
#include "Bit_Math.h"

#include "ADC_Registers.h"
#include "ADC_Interface.h"

/* Initialise the ADC peripheral with the configuration described above. */
void ADC_voidInit(void)
{
	/* Voltage reference: AREF (bits REFS1:REFS0 = 00) */
	ADMUX = AREF << 6;

	/* Right-align result (ADLAR bit = 0): 10-bit result in ADCL (low) + ADCH (high) */
	ADMUX |= ADCLEFT_ADJUSTMENT_OFF << 5;

	/* Enable Auto-Trigger (free-running mode) */
	Set_Bit(ADCSRA, ADCSRA_ADATE);

	/* Enable the ADC module */
	Set_Bit(ADCSRA, ADCSRA_ADEN);

	/* Set prescaler to DIV_64: ADC clock = 8 MHz / 64 = 125 kHz */
	ADCSRA |= DIV_64 << 0;

	/* Clear the ADC Interrupt Flag (write 1 to clear) to start clean */
	Set_Bit(ADCSRA, ADCSRA_ADIF);
}

/* Select the ADC channel, start a single conversion, wait for it to complete,
 * clear the interrupt flag, and return the 10-bit result.
 *
 * ADCLH is a 16-bit register alias that combines ADCL and ADCH; reading it
 * returns the right-aligned 10-bit conversion result (0–1023). */
u16 ADC_u16Read(ADC_CHANNELS_options_t Copy_u8ADCChannel)
{
	/* Clear channel bits (MUX4:MUX0) then select the requested channel */
	ADMUX &= 0b11100000;
	ADMUX |= Copy_u8ADCChannel << 0;

	/* Start a single ADC conversion */
	Set_Bit(ADCSRA, ADCSRA_ADSC);

	/* Wait until the conversion is complete (ADIF flag set by hardware) */
	while (!Get_Bit(ADCSRA, ADCSRA_ADIF));

	/* Clear ADIF by writing 1 so the next conversion can be detected */
	Set_Bit(ADCSRA, ADCSRA_ADIF);

	/* Return the 10-bit conversion result */
	return ADCLH;
}

/* Convert a raw 10-bit ADC value to the corresponding voltage in volts.
 * Formula: V = (ADC_raw / 1024) × VREF
 * Example: ADC_raw = 512, VREF = 5 V → V = 2.5 V */
f32 ADC_f32ADCtoVolt(u16 Copy_u16ADCVal)
{
	f32 ADC_Localf32Volt = (Copy_u16ADCVal / 1024.0) * VREF;
	return ADC_Localf32Volt;
}
