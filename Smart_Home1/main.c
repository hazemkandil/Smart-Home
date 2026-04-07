/*
 * main.c  –  Smart_Home1 (ATmega32 – Keypad / Transmitter MCU)
 *
 *  Created on: Sep 19, 2022
 *      Author: Hazem Mohamed
 *
 * -----------------------------------------------------------------------
 *  SYSTEM OVERVIEW
 * -----------------------------------------------------------------------
 *  Smart_Home1 is the user-input node of a two-MCU Smart Home system.
 *  It is connected to Smart_Home2 (ATmega16) via a USART serial link.
 *
 *  Responsibilities:
 *    1. Read digit keypresses from a 4×3 matrix keypad.
 *    2. Transmit the digits (username + password) to Smart_Home2.
 *    3. Receive control tokens from Smart_Home2 and update PORTB LEDs
 *       to reflect the current authentication state:
 *         '-'  → 1st attempt  → PORTB = THREE  (segment display "3")
 *         'W'  → 1st failure  → PORTB = TWO    (segment display "2")
 *         'N'  → 2nd failure  → PORTB = ONE    (segment display "1")
 *         'T'  → system locked → PORTB = ZERO  (segment display "0")
 *                then waits for '.' confirmation before re-enabling INT0
 *         'D'  → success      → PORTB = LOW    (all segments off)
 *
 *    4. Respond to the INT0 external interrupt (falling-edge on PD2):
 *         The Enable() callback disables INT0 and sends '/' to Smart_Home2
 *         to signal that a button press was detected and the login flow
 *         should begin.
 *
 *  Hardware connections (ATmega32):
 *    PORTD PIN2 (PD2) – INT0 input, active-low push-button (pull-up enabled)
 *    PORTD PIN0       – USART RX (input)
 *    PORTD PIN1       – USART TX (output)
 *    PORTB            – 7-segment / LED display (output)
 *    PORTC            – Keypad rows and columns (configured in KeyPad_Config.h)
 * -----------------------------------------------------------------------
 */

#include "Std_Types.h"
#include "Bit_Math.h"

#include "DIO_Interface.h"

#include "EXTI_Interface.h"

#include "USART_Interface.h"

#include "KeyPad_Interface.h"

#include "util/delay.h"

#include "APP_Interface.h"

/* Forward declaration of the INT0 callback defined at the bottom of this file */
void Enable(void);

int main(void)
{
	/* ---- Peripheral Initialisation ---- */
	USART_voidInit();           /* 9600 bps, 8N1, TX+RX enabled                */

	EXTI_voidGlobalEnable();    /* set I-bit in SREG                            */
	EXTI_voidINT0Init();        /* INT0 on PD2, falling-edge, enables interrupt  */

	KeyPad_voidInit();          /* configure keypad rows (output) / cols (input) */

	/* PD2 = INT0 button input with internal pull-up */
	DIO_voidSetPinDirection(DIO_PORTD, PIN2, INPUT_PIN);
	DIO_voidPullUpPin      (DIO_PORTD, PIN2);

	/* USART pins: PD0 = RX (input), PD1 = TX (output) */
	DIO_voidSetPinDirection(DIO_PORTD, PIN0, INPUT_PIN);
	DIO_voidSetPinDirection(DIO_PORTD, PIN1, OUTPUT_PIN);

	/* PORTB drives a 7-segment / LED display showing attempt count */
	DIO_voidSetPortDirection(DIO_PORTB, OUTPUT_PORT);

	/* Register the Enable() function as the INT0 callback.
	 * When the door-bell button is pressed, Enable() fires, disables INT0
	 * (preventing re-entry), and sends '/' to start the login flow. */
	EXTI_voidCallBackINT0(&Enable);

	/* ---- Main Loop ---- */
	while (1)
	{
		/* Blocking read: wait for a control token from Smart_Home2 */
		u8 Read = USART_u8ReadData();

		if (Read == '-')
		{
			/* 1st attempt – show "3 remaining attempts" on display */
			DIO_voidSetPortValue(DIO_PORTB, THREE);
		}
		else if (Read == 'W')
		{
			/* 1st wrong attempt – show "2 remaining attempts" */
			DIO_voidSetPortValue(DIO_PORTB, TWO);
		}
		else if (Read == 'N')
		{
			/* 2nd wrong attempt – show "1 remaining attempt" */
			DIO_voidSetPortValue(DIO_PORTB, ONE);
		}
		else if (Read == 'D')
		{
			/* Credentials accepted – blank the display */
			DIO_voidSetPortValue(DIO_PORTB, LOW);
		}
		else if (Read == 'T')
		{
			/* System locked (3rd failure) – show "0" on display */
			DIO_voidSetPortValue(DIO_PORTB, ZERO);

			/* Wait for the '.' confirmation that Smart_Home2 has
			 * fully applied the lockout before re-enabling INT0 */
			if (USART_u8ReadData() == '.')
				DIO_voidSetPortValue(DIO_PORTB, LOW);

			EXTI_voidINT0Init(); /* re-enable INT0 for next door-bell press */
			continue;            /* skip credential collection this cycle   */
		}

		/* Collect 8 digits: 4 username + 4 password, one per loop iteration.
		 * Each call blocks until the user presses a valid numeric key. */
		for (u8 Index = 0; Index < 8; Index++)
			APP_voidCheckUserPassword();
	}

	return 0;
}

/* INT0 Callback – fired when the door-bell button on PD2 is pressed.
 *
 * Actions:
 *   1. Disable INT0 to prevent repeated triggers during the login flow.
 *   2. Transmit '/' to Smart_Home2 to signal the start of authentication. */
void Enable(void)
{
	EXTI_voidINT0End();          /* disable INT0 – re-enabled after lockout/success */
	USART_voidWriteData('/');    /* notify Smart_Home2: door-bell was pressed        */
}
