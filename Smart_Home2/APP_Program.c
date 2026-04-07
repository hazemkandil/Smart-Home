/*
 * APP_Program.c  –  Smart_Home2 (ATmega16 – display/control MCU)
 *
 *  Created on: Sep 19, 2022
 *      Author: Hazem Mohamed
 *
 *  Description:
 *      Application-layer helpers for the display/control MCU.
 *      Handles incoming credential bytes from Smart_Home1 (via USART)
 *      and provides credential comparison functions.
 *
 *      Credential display contracts:
 *        - Username digits   → shown as plain characters on LCD line 2
 *        - Password digits   → shown briefly as the character, then
 *                              replaced with '*' after 200 ms (masking)
 *        - Both wrap back to column 0 after 4 digits (CREDENTIAL_LENGTH)
 */

#include "Std_Types.h"
#include "Bit_Math.h"

#include "LCD_Interface.h"

#include "APP_Interface.h"

#include <util/delay.h>

/* Display the received username digit on LCD line 2 at the current
 * column position, then advance the column (wraps at CREDENTIAL_LENGTH).
 * Uses a static index so each successive call moves one column right. */
void APP_voidReceiveUser(u8 Copy_Num)
{
	static u8 Index = 0;
	LCD_voidGoToXY(Index, 1);      /* position cursor on line 2 */
	LCD_voidSendData(Copy_Num);    /* display the digit         */
	Index++;
	if (Index == 4)                /* reset after 4 digits      */
		Index = 0;
}

/* Display the received password digit on LCD line 2, wait 200 ms so
 * the user can see what they typed, then overwrite it with '*' for masking.
 * Uses a static index that wraps at CREDENTIAL_LENGTH. */
void APP_voidReceivePassword(u8 Copy_Num)
{
	static u8 Index = 0;
	LCD_voidGoToXY(Index, 1);      /* position cursor on line 2   */
	LCD_voidSendData(Copy_Num);    /* briefly show the digit       */
	_delay_ms(200);                /* 200 ms visibility window     */
	LCD_voidGoToXY(Index, 1);      /* return to same position      */
	LCD_voidSendData('*');         /* replace digit with mask char */
	Index++;
	if (Index == 4)                /* reset after 4 digits         */
		Index = 0;
}

/* Compare two 4-byte credential arrays element by element.
 * Returns 1 (match) if all bytes are equal, 0 (mismatch) on first difference.
 *
 * ArrTrue  – the stored correct credential (DEFAULT_USER or DEFAULT_PASS)
 * ArrCheck – the credential entered by the user this attempt */
u8 APP_u8UserCheck(u8 *ArrTrue, u8 *ArrCheck)
{
	for (u8 Index = 0; Index < 4; Index++)
		if (ArrTrue[Index] != ArrCheck[Index])
			return 0;  /* mismatch found */
	return 1;          /* all bytes matched */
}

/* Identical comparison logic to APP_u8UserCheck; kept separate to allow
 * independent password-specific validation rules in future revisions. */
u8 APP_u8PassCheck(u8 *ArrTrue, u8 *ArrCheck)
{
	for (u8 Index = 0; Index < 4; Index++)
		if (ArrTrue[Index] != ArrCheck[Index])
			return 0;
	return 1;
}
