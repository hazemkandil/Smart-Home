/*
 * USART_Program.c
 *
 *  Created on: Sep 11, 2022
 *      Author: Hazem Mohamed
 *
 *  Description:
 *      USART driver for the ATmega16 (Smart_Home2 – display/control MCU).
 *      Configures asynchronous serial communication at 9600 bps,
 *      8-bit data, 1 stop bit, no parity.
 *
 *      Smart_Home2 uses USART to receive digit characters entered on
 *      the keypad by Smart_Home1, and to send control tokens back:
 *        '-' → first attempt, prompt Smart_Home1 to collect credentials
 *        'W' → 1st wrong attempt warning
 *        'N' → 2nd wrong attempt warning
 *        'T' → system locked (3rd failure) – also sends '.' to confirm lock
 *        'D' → credentials accepted, begin door-open PWM sweep
 */

#include "Std_Types.h"
#include "Bit_Math.h"

#include "USART_Interface.h"
#include "USART_Registers.h"

void USART_voidInit()
{
	/* Character size: clear UCSZ2 bit in UCSRB so that
	 * the 8-bit frame size is selected together with UCSZ1:UCSZ0 below */
	Clear_Bit(UCSRB, UCSRB_UCSZ2);

	/* UCSRC configuration:
	 *   Bit 7 (URSEL)  = 1  → select UCSRC register (shared address with UBRRH)
	 *   Bits 5:4 (UPM) = 00 → no parity
	 *   Bit  3 (USBS)  = 0  → 1 stop bit
	 *   Bits 2:1 (UCSZ)= 11 → 8-bit character size
	 *   Bit  0 (UCPOL) = 0  → no clock polarity (async mode) */
	UCSRC = 0b10000110;

	/* Baud rate = 9600 bps @ 8 MHz → UBRR = (8000000 / (16*9600)) - 1 = 51 */
	UBRRL = 51;

	/* Enable Transmitter and Receiver */
	Set_Bit(UCSRB, UCSRB_TXEN);
	Set_Bit(UCSRB, UCSRB_RXEN);
}

/* Transmit one byte over USART (blocking).
 * Waits until the transmit buffer is empty before loading new data. */
void USART_voidWriteData(u8 Copy_u8Data)
{
	/*Waiting until Transmit Buffer will be empty and ready for new data*/
	while (Get_Bit(UCSRA, UCSRA_UDRE) != 1);

	/*Put New Data*/
	UDR = Copy_u8Data;
}

/* Receive one byte from USART (blocking).
 * Waits until a complete frame has been received before reading. */
u8 USART_u8ReadData()
{
	/*Waiting until Receive Buffer to finish receiving and to be ready for new data*/
	while (Get_Bit(UCSRA, UCSRA_RXC) != 1);

	/*Read New Data*/
	return UDR;
}
