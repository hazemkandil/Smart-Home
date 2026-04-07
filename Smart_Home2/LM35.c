/*
 * LM35.c
 *
 *  Created on: Sep 6, 2022
 *      Author: Hazem Mohamed
 *
 *  Description:
 *      LM35 temperature sensor processing module for Smart_Home2.
 *      The LM35 outputs 10 mV/°C, so:
 *          Temperature (°C) = ADC_Voltage (V) × 100
 *
 *      This module provides three functions:
 *        1. LM35_voidDisplayBits       – debug helper (currently unused/commented)
 *                                        displays the raw 10-bit ADC value in binary
 *        2. LM35_voidDisplayVolt       – displays a float as "XX.X" on the LCD
 *        3. LM35_voidVoltToTemperature – converts ADC voltage to °C and displays it
 *
 *      In main.c the temperature controls the fan output (PORTA PIN3):
 *        ≥ 26.00 °C → fan ON  + motor icon displayed at LCD column 15
 *        < 26.00 °C → fan OFF + blank icon at LCD column 15
 */

#include "Std_Types.h"
#include "Bit_Math.h"

#include "ADC_Registers.h"
#include "ADC_Interface.h"

#include "LCD_Interface.h"
#include "LCD_Configure.h"

#include "DIO_Interface.h"

#include "LM35.h"

//u8 Global_u8Delete = 0;  /* debug remnant – not used */

/* [Debug / Not used in production]
 * Read the raw 10-bit ADC value and display each bit on the LCD MSB-first.
 * Useful during development to verify ADC readings before voltage conversion. */
void LM35_voidDisplayBits(ADC_CHANNELS_options_t Copy_u8ADCChannel)
{
	u16 Local_u16Bits  = ADC_u16Read(Copy_u8ADCChannel);
	u8  Local_u16Index = 1;

	/* Iterate 10 times (10 ADC bits), masking the MSB each iteration */
	while(Local_u16Index <= 10)
	{
		if (Local_u16Bits & 0b1000000000)
			LCD_voidWriteNumber(1);
		else
			LCD_voidWriteNumber(0);

		Local_u16Bits = Local_u16Bits << 1;  /* shift next bit into position */
		Local_u16Index++;
	}
}

/* Display a floating-point voltage/temperature value on the LCD in "XX.X" format.
 * Example: 26.3 → integer part (26) displayed digit by digit,
 *                  decimal point,
 *                  one fractional digit (3).
 *
 * Algorithm (avoids floating-point division on AVR):
 *   intPart   = (u16) value          → e.g. 26
 *   floatPart = (u16)(value * 10.0)  → e.g. 263  →  %10 → 3  (last digit) */
void LM35_voidDisplayVolt(f32 Local_f32Volt)
{
	u16 Local_u8IntPart   = Local_f32Volt;         /* integer portion            */
	u16 Local_u8FloatPart = Local_f32Volt * 10.0;  /* shift one decimal place    */
	u8  Local_u8Index     = 255;                   /* wraps to 0 after first ++  */
	u8  Local_u8Arr[5];                            /* digit buffer (up to 5 digits) */

	if (Local_u8IntPart == 0)
	{
		LCD_voidWriteNumber(0);  /* edge case: display single '0' */
	}
	else
	{
		/* Extract integer digits into buffer (stored in reverse order) */
		while (Local_u8IntPart)
		{
			Local_u8Index++;
			Local_u8Arr[Local_u8Index] = Local_u8IntPart % 10;  /* least-significant digit */
			Local_u8IntPart /= 10;
		}

		/* Print digits in correct order (most-significant first) */
		for (u8 Local_u8Counter = Local_u8Index; Local_u8Counter != 255; Local_u8Counter--)
		{
			LCD_voidWriteNumber(Local_u8Arr[Local_u8Counter]);
		}
	}

	LCD_voidSendData('.');                      /* decimal point separator  */

	Local_u8FloatPart %= 10;                    /* extract single fractional digit */
	LCD_voidWriteNumber(Local_u8FloatPart);
}

/* Convert ADC voltage to temperature (°C) using LM35 sensitivity (10 mV/°C):
 *     Temperature = Voltage × 100
 *
 * Also handles LCD rendering:
 *   - calls LM35_voidDisplayVolt() to print the value
 *   - appends the degree symbol (0xDF) and 'C'
 *
 * Returns the temperature as a float for use in the fan control logic. */
f32 LM35_voidVoltToTemperature(f32 Copy_f32ADCVolt)
{
	f32 Local_f32Temp = Copy_f32ADCVolt * 100; /* 10 mV/°C → multiply by 100 */

	LM35_voidDisplayVolt(Local_f32Temp);    /* render temperature on LCD    */
	LCD_voidSendData(0b11011111);           /* '°' degree symbol (LCD ROM)  */
	LCD_voidSendData('C');                  /* unit label                   */

	return Local_f32Temp;
}
