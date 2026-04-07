/*
 * DIO_Program.c  –  Smart_Home1 (ATmega32 – keypad MCU)
 *
 *  Created on: Aug 30, 2022
 *      Author: Hazem Mohamed
 *
 *  Description:
 *      Digital I/O (DIO) driver – see Smart_Home2/DIO_Program.c for
 *      full description.  This copy targets the ATmega32 and its
 *      identical register layout (PORTA–PORTD, PINA–PIND, DDRA–DDRD).
 *
 *      In Smart_Home1 the DIO driver is used to:
 *        - Configure PORTB as output for 7-segment/LED display
 *        - Configure PD2 as input + pull-up for the INT0 door-bell button
 *        - Configure PD0/PD1 for USART RX/TX direction
 *        - Manage keypad row/column pin directions (via KeyPad_Program.c)
 */

#include "Bit_Math.h"
#include "Std_Types.h"

#include "DIO_Interface.h"
#include "DIO_Registers.h"

/* Set the direction of an entire 8-pin port.
 * OUTPUT_PORT (0xFF) → all pins output
 * INPUT_PORT  (0x00) → all pins input (high-Z) */
void DIO_voidSetPortDirection(u8 Copy_u8Port, u8 Copy_u8Direction) {
	switch (Copy_u8Port) {
	case DIO_PORTA: DDRA = Copy_u8Direction; break;
	case DIO_PORTB: DDRB = Copy_u8Direction; break;
	case DIO_PORTC: DDRC = Copy_u8Direction; break;
	case DIO_PORTD: DDRD = Copy_u8Direction; break;
	default: break;
	}
}

/* Set the direction of a single pin.
 * OUTPUT_PIN → sets the DDR bit  (1 = output)
 * INPUT_PIN  → clears the DDR bit (0 = input, high-Z) */
void DIO_voidSetPinDirection(u8 Copy_u8Port, u8 Copy_u8Pin,
                              u8 Copy_u8Direction) {
	if (Copy_u8Direction == OUTPUT_PIN) {
		switch (Copy_u8Port) {
		case DIO_PORTA: Set_Bit(DDRA, Copy_u8Pin); break;
		case DIO_PORTB: Set_Bit(DDRB, Copy_u8Pin); break;
		case DIO_PORTC: Set_Bit(DDRC, Copy_u8Pin); break;
		case DIO_PORTD: Set_Bit(DDRD, Copy_u8Pin); break;
		default: break;
		}
	} else if (Copy_u8Direction == INPUT_PIN) {
		switch (Copy_u8Port) {
		case DIO_PORTA: Clear_Bit(DDRA, Copy_u8Pin); break;
		case DIO_PORTB: Clear_Bit(DDRB, Copy_u8Pin); break;
		case DIO_PORTC: Clear_Bit(DDRC, Copy_u8Pin); break;
		case DIO_PORTD: Clear_Bit(DDRD, Copy_u8Pin); break;
		default: break;
		}
	}
}

/* Write a value to an entire output port. */
void DIO_voidSetPortValue(u8 Copy_u8Port, u8 Copy_u8Value) {
	switch (Copy_u8Port) {
	case DIO_PORTA: PORTA = Copy_u8Value; break;
	case DIO_PORTB: PORTB = Copy_u8Value; break;
	case DIO_PORTC: PORTC = Copy_u8Value; break;
	case DIO_PORTD: PORTD = Copy_u8Value; break;
	default: break;
	}
}

/* Set or clear a single output pin.
 * HIGH → Set_Bit (drives to VCC) | LOW → Clear_Bit (drives to GND) */
void DIO_voidSetPinValue(u8 Copy_u8Port, u8 Copy_u8Pin, u8 Copy_u8Value) {
	if (Copy_u8Value == HIGH) {
		switch (Copy_u8Port) {
		case DIO_PORTA: Set_Bit(PORTA, Copy_u8Pin); break;
		case DIO_PORTB: Set_Bit(PORTB, Copy_u8Pin); break;
		case DIO_PORTC: Set_Bit(PORTC, Copy_u8Pin); break;
		case DIO_PORTD: Set_Bit(PORTD, Copy_u8Pin); break;
		default: break;
		}
	} else if (Copy_u8Value == LOW) {
		switch (Copy_u8Port) {
		case DIO_PORTA: Clear_Bit(PORTA, Copy_u8Pin); break;
		case DIO_PORTB: Clear_Bit(PORTB, Copy_u8Pin); break;
		case DIO_PORTC: Clear_Bit(PORTC, Copy_u8Pin); break;
		case DIO_PORTD: Clear_Bit(PORTD, Copy_u8Pin); break;
		default: break;
		}
	}
}

/* Read a single input pin from the PINx register.
 * Returns 1 (HIGH) or 0 (LOW). */
u8 DIO_u8GetPinValue(u8 Copy_u8Port, u8 Copy_u8Pin) {
	u8 Local_u8ReturnValuePin;
	switch (Copy_u8Port) {
	case DIO_PORTA: Local_u8ReturnValuePin = Get_Bit(PINA, Copy_u8Pin); break;
	case DIO_PORTB: Local_u8ReturnValuePin = Get_Bit(PINB, Copy_u8Pin); break;
	case DIO_PORTC: Local_u8ReturnValuePin = Get_Bit(PINC, Copy_u8Pin); break;
	case DIO_PORTD: Local_u8ReturnValuePin = Get_Bit(PIND, Copy_u8Pin); break;
	default: break;
	}
	return Local_u8ReturnValuePin;
}

/* Enable the internal pull-up resistor on an input pin.
 * On AVR, writing HIGH to PORTx while DDRx bit = 0 enables the pull-up.
 * Used for the INT0 door-bell button on PD2. */
void DIO_voidPullUpPin(u8 Copy_u8Port, u8 Copy_u8Pin) {
	switch (Copy_u8Port) {
	case DIO_PORTA: Set_Bit(PORTA, Copy_u8Pin); break;
	case DIO_PORTB: Set_Bit(PORTB, Copy_u8Pin); break;
	case DIO_PORTC: Set_Bit(PORTC, Copy_u8Pin); break;
	case DIO_PORTD: Set_Bit(PORTD, Copy_u8Pin); break;
	default: break;
	}
}
