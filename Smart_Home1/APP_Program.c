/*
 * APP_Program.c  –  Smart_Home1 (ATmega32 – keypad MCU)
 *
 *  Created on: Sep 19, 2022
 *      Author: Hazem Mohamed
 *
 *  Description:
 *      Application-layer logic for the keypad MCU.
 *      Provides a single function that blocks until the user presses
 *      a numeric digit (0-9) on the keypad, then transmits it to
 *      Smart_Home2 over USART.
 *
 *      Called 8 times per login attempt from main.c (4 username digits
 *      + 4 password digits), with a 30 ms inter-character delay to
 *      allow Smart_Home2's blocking USART_u8ReadData() to catch each byte.
 */

#include "Std_Types.h"
#include "Bit_Math.h"

#include "DIO_Interface.h"

#include "EXTI_Interface.h"

#include "USART_Interface.h"

#include "KeyPad_Interface.h"

#include "util/delay.h"

/* Wait for the user to press a valid numeric digit (0-9) on the keypad,
 * then send the corresponding ASCII character to Smart_Home2 via USART.
 *
 * Non-digit keys (letters, '#', '*') are ignored and the keypad is
 * polled again until a digit is detected.
 * A 30 ms delay after each transmission gives Smart_Home2 time to
 * read the byte before the next one is sent. */
void APP_voidCheckUserPassword (void)
{
	u8 Local_u8PassNumber = KeyPad_u8GetPress();

	/* Keep reading until a valid digit (ASCII '0'–'9') is pressed */
	while (Local_u8PassNumber < '0' || Local_u8PassNumber > '9')
	{
		Local_u8PassNumber = KeyPad_u8GetPress();
	}

	/* Forward the digit character directly over USART.
	 * A switch is used (rather than USART_voidWriteData(Local_u8PassNumber))
	 * to allow per-digit processing hooks in the future if needed. */
	switch (Local_u8PassNumber)
	{
	case '0': USART_voidWriteData('0'); _delay_ms(30); break;
	case '1': USART_voidWriteData('1'); _delay_ms(30); break;
	case '2': USART_voidWriteData('2'); _delay_ms(30); break;
	case '3': USART_voidWriteData('3'); _delay_ms(30); break;
	case '4': USART_voidWriteData('4'); _delay_ms(30); break;
	case '5': USART_voidWriteData('5'); _delay_ms(30); break;
	case '6': USART_voidWriteData('6'); _delay_ms(30); break;
	case '7': USART_voidWriteData('7'); _delay_ms(30); break;
	case '8': USART_voidWriteData('8'); _delay_ms(30); break;
	case '9': USART_voidWriteData('9'); _delay_ms(30); break;
	}
}
