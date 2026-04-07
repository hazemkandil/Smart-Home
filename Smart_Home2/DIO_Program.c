/*
 * DIO_Program.c  –  Smart_Home1 & Smart_Home2 (shared driver)
 *
 *  Created on: Aug 30, 2022
 *      Author: Hazem Mohamed
 *
 *  Description:
 *      Digital I/O (DIO) driver for the ATmega32/ATmega16.
 *      Provides a hardware-abstraction layer over the AVR port registers
 *      (DDRx, PORTx, PINx) so higher-level code never writes to registers
 *      directly and port assignments can be changed in one place.
 *
 *      Port identifiers (defined in DIO_Interface.h):
 *        DIO_PORTA = 0, DIO_PORTB = 1, DIO_PORTC = 2, DIO_PORTD = 3
 *
 *      Pin identifiers: PIN0 – PIN7
 *
 *      Direction values: OUTPUT_PIN = 1, INPUT_PIN = 0
 *      Logic values:     HIGH = 1,       LOW = 0
 *
 *      Note – DIO_voidPullUpPin():
 *        On AVR, writing HIGH to a pin that is configured as INPUT enables
 *        the internal pull-up resistor.  This is used for the INT0 button
 *        (PD2) and keypad column pins.
 */

#include "Std_Types.h"
#include "Bit_Math.h"

#include "DIO_Interface.h"
#include "DIO_Registers.h"

/* Set the direction of an entire 8-pin port at once.
 * Copy_u8Direction: OUTPUT_PORT (0xFF) or INPUT_PORT (0x00). */
void DIO_voidSetPortDirection(u8 Copy_u8Port, u8 Copy_u8Direction)
{
	switch (Copy_u8Port)
	{
	case DIO_PORTA: DDRA = Copy_u8Direction; break;
	case DIO_PORTB: DDRB = Copy_u8Direction; break;
	case DIO_PORTC: DDRC = Copy_u8Direction; break;
	case DIO_PORTD: DDRD = Copy_u8Direction; break;
	default: break;
	}
}

/* Set the direction of a single pin within a port.
 * OUTPUT_PIN → sets the DDR bit  (1 = output)
 * INPUT_PIN  → clears the DDR bit (0 = input, high-Z) */
void DIO_voidSetPinDirection(u8 Copy_u8Port, u8 Copy_u8Pin, u8 Copy_u8Direction)
{
	if (Copy_u8Direction == OUTPUT_PIN)
	{
		switch (Copy_u8Port)
		{
		case DIO_PORTA: Set_Bit(DDRA, Copy_u8Pin); break;
		case DIO_PORTB: Set_Bit(DDRB, Copy_u8Pin); break;
		case DIO_PORTC: Set_Bit(DDRC, Copy_u8Pin); break;
		case DIO_PORTD: Set_Bit(DDRD, Copy_u8Pin); break;
		default: break;
		}
	}
	else if (Copy_u8Direction == INPUT_PIN)
	{
		switch (Copy_u8Port)
		{
		case DIO_PORTA: Clear_Bit(DDRA, Copy_u8Pin); break;
		case DIO_PORTB: Clear_Bit(DDRB, Copy_u8Pin); break;
		case DIO_PORTC: Clear_Bit(DDRC, Copy_u8Pin); break;
		case DIO_PORTD: Clear_Bit(DDRD, Copy_u8Pin); break;
		default: break;
		}
	}
}

/* Write a value to an entire output port (e.g., LED pattern on PORTB). */
void DIO_voidSetPortValue(u8 Copy_u8Port, u8 Copy_u8Value)
{
	switch (Copy_u8Port)
	{
	case DIO_PORTA: PORTA = Copy_u8Value; break;
	case DIO_PORTB: PORTB = Copy_u8Value; break;
	case DIO_PORTC: PORTC = Copy_u8Value; break;
	case DIO_PORTD: PORTD = Copy_u8Value; break;
	default: break;
	}
}

/* Set or clear a single output pin.
 * HIGH → Set_Bit   (drives pin to VCC)
 * LOW  → Clear_Bit (drives pin to GND) */
void DIO_voidSetPinValue(u8 Copy_u8Port, u8 Copy_u8Pin, u8 Copy_u8Value)
{
	if (Copy_u8Value == HIGH)
	{
		switch (Copy_u8Port)
		{
		case DIO_PORTA: Set_Bit(PORTA, Copy_u8Pin); break;
		case DIO_PORTB: Set_Bit(PORTB, Copy_u8Pin); break;
		case DIO_PORTC: Set_Bit(PORTC, Copy_u8Pin); break;
		case DIO_PORTD: Set_Bit(PORTD, Copy_u8Pin); break;
		default: break;
		}
	}
	else if (Copy_u8Value == LOW)
	{
		switch (Copy_u8Port)
		{
		case DIO_PORTA: Clear_Bit(PORTA, Copy_u8Pin); break;
		case DIO_PORTB: Clear_Bit(PORTB, Copy_u8Pin); break;
		case DIO_PORTC: Clear_Bit(PORTC, Copy_u8Pin); break;
		case DIO_PORTD: Clear_Bit(PORTD, Copy_u8Pin); break;
		default: break;
		}
	}
}

/* Read the logic level of a single input pin from the PINx register.
 * Returns 1 (HIGH) or 0 (LOW). */
u8 DIO_u8GetPinValue(u8 Copy_u8Port, u8 Copy_u8Pin)
{
	u8 Local_u8ReturnValuePin;
	switch (Copy_u8Port)
	{
	case DIO_PORTA: Local_u8ReturnValuePin = Get_Bit(PINA, Copy_u8Pin); break;
	case DIO_PORTB: Local_u8ReturnValuePin = Get_Bit(PINB, Copy_u8Pin); break;
	case DIO_PORTC: Local_u8ReturnValuePin = Get_Bit(PINC, Copy_u8Pin); break;
	case DIO_PORTD: Local_u8ReturnValuePin = Get_Bit(PIND, Copy_u8Pin); break;
	default: break;
	}
	return Local_u8ReturnValuePin;
}

/* Enable the internal pull-up resistor on an input pin.
 * On AVR, writing HIGH to PORTx while the pin is INPUT enables the pull-up.
 * Used for: INT0 button (PD2) and keypad column input lines. */
void DIO_voidPullUpPin(u8 Copy_u8Port, u8 Copy_u8Pin)
{
	switch (Copy_u8Port)
	{
	case DIO_PORTA: Set_Bit(PORTA, Copy_u8Pin); break;
	case DIO_PORTB: Set_Bit(PORTB, Copy_u8Pin); break;
	case DIO_PORTC: Set_Bit(PORTC, Copy_u8Pin); break;
	case DIO_PORTD: Set_Bit(PORTD, Copy_u8Pin); break;
	default: break;
	}
}
