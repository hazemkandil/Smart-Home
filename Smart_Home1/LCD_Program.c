/*
 * LCD_Program.c
 *
 *  Created on: Aug 31, 2022
 *      Author: Hazem Mohamed
 *
 *  Description:
 *      HD44780-compatible 16×2 LCD driver operating in 8-bit parallel mode.
 *      Used by both Smart_Home1 and Smart_Home2 to display status messages
 *      such as "Enter User:", "Wrong Data", "Opening Door...", and live
 *      sensor readings (temperature, LED count).
 *
 *      Control lines (defined in LCD_Configure.h):
 *        RS  – Register Select: LOW = command, HIGH = data
 *        RW  – Read/Write:      always LOW (write-only mode)
 *        E   – Enable:          pulsed HIGH→LOW to latch each byte
 *
 *      Data lines: full 8-bit port (LCD_DATA_PORT).
 */

#include "STD_TYPES.h"
#include "BIT_MATH.h"

#include "DIO_Interface.h"

#include "LCD_Configure.h"
#include "LCD_Interface.h"

#include <util/delay.h>

/* Send a command byte to the LCD (RS=LOW, RW=LOW).
 * E is pulsed HIGH for 2 ms then LOW to latch the command.
 * Common commands: 0x01 = clear screen, 0x0C = display on/cursor off. */
void LCD_voidSendCommand(u8 Copy_u8Command)
{
	DIO_voidSetPinValue(LCD_CTRL_PORT, LCD_RS_PIN, LOW);
	/* Set RS LOW → command register */

	DIO_voidSetPinValue(LCD_CTRL_PORT, LCD_RW_PIN, LOW);
	/* Set RW LOW → write mode */

	DIO_voidSetPortValue(LCD_DATA_PORT, Copy_u8Command);
	/* Place command on data bus */

	DIO_voidSetPinValue(LCD_CTRL_PORT, LCD_E_PIN, HIGH);
	_delay_ms(2);
	DIO_voidSetPinValue(LCD_CTRL_PORT, LCD_E_PIN, LOW);
	/* Pulse Enable to latch command into LCD */
}

/* Send a data byte (character) to the LCD (RS=HIGH, RW=LOW).
 * E is pulsed HIGH for 2 ms then LOW to latch the character.  */
void LCD_voidSendData(u8 Copy_u8Data)
{
	DIO_voidSetPinValue(LCD_CTRL_PORT, LCD_RS_PIN, HIGH);
	/* Set RS HIGH → data register */

	DIO_voidSetPinValue(LCD_CTRL_PORT, LCD_RW_PIN, LOW);
	/* Set RW LOW → write mode */

	DIO_voidSetPortValue(LCD_DATA_PORT, Copy_u8Data);
	/* Place character on data bus */

	DIO_voidSetPinValue(LCD_CTRL_PORT, LCD_E_PIN, HIGH);
	_delay_ms(2);
	DIO_voidSetPinValue(LCD_CTRL_PORT, LCD_E_PIN, LOW);
	/* Pulse Enable to latch character into LCD */
}

/* Initialise the LCD after power-on.
 * Sequence: wait 35 ms → function set → display on → clear screen. */
void LCD_voidInit()
{
	/* Wait after power-on for the LCD controller to stabilise (≥15 ms typical) */
	_delay_ms(35);

	/* Function Set: 8-bit interface, 2 display lines, 5×8 dot font */
	LCD_voidSendCommand(0b00111100);

	/* Display ON, cursor OFF, blink OFF */
	LCD_voidSendCommand(0b00001100);

	/* Clear Display (also resets DDRAM address counter to 0) */
	LCD_voidSendCommand(0b00000001);
}

/* Configure the data port as output and the three control pins as output.
 * Must be called before LCD_voidInit(). */
void LCD_voidConfigDirection()
{
	/* Set entire data port as output */
	DIO_voidSetPortDirection(LCD_DATA_PORT,  OUTPUT_PORT);
	/* Set control pins as output */
	DIO_voidSetPinDirection (LCD_CTRL_PORT, LCD_RS_PIN, HIGH);
	DIO_voidSetPinDirection (LCD_CTRL_PORT, LCD_RW_PIN, HIGH);
	DIO_voidSetPinDirection (LCD_CTRL_PORT, LCD_E_PIN,  HIGH);
}

/* Write a null-terminated string to the LCD at the current cursor position. */
void LCD_voidWriteString(s8 *Copy_u8String)
{
	u8 Local_Index = 0;
	while (Copy_u8String[Local_Index] != 0)   /* iterate until null terminator */
	{
		LCD_voidSendData(Copy_u8String[Local_Index]);
		Local_Index++;
	}
}

/* Write a single decimal digit (0–9) to the LCD as its ASCII character.
 * Digits outside this range are silently ignored (default case). */
void LCD_voidWriteNumber(u8 Copy_u8Number)
{
	switch (Copy_u8Number)
	{
	case 0:  LCD_voidSendData('0'); break;
	case 1:  LCD_voidSendData('1'); break;
	case 2:  LCD_voidSendData('2'); break;
	case 3:  LCD_voidSendData('3'); break;
	case 4:  LCD_voidSendData('4'); break;
	case 5:  LCD_voidSendData('5'); break;
	case 6:  LCD_voidSendData('6'); break;
	case 7:  LCD_voidSendData('7'); break;
	case 8:  LCD_voidSendData('8'); break;
	case 9:  LCD_voidSendData('9'); break;
	default: break;
	}
}

/* Move the cursor to column X (0-based) on line Y (0 = line 1, 1 = line 2).
 * Computes the DDRAM address and sends a Set-DDRAM-Address command (bit 7 = 1).
 *   Line 1: DDRAM = X          (0x00 – 0x0F)
 *   Line 2: DDRAM = X + 0x40   (0x40 – 0x4F) */
void LCD_voidGoToXY(u8 Copy_u8XPos, u8 Copy_u8YPos)
{
	u8 Local_u8DDRAM_Address;

	if (Copy_u8YPos == LCD_LINE_ONE)
	{
		Local_u8DDRAM_Address = Copy_u8XPos;
	}
	else if (Copy_u8YPos == LCD_LINE_TWO)
	{
		Local_u8DDRAM_Address = Copy_u8XPos + 0x40; /* line 2 base address offset */
	}

	Local_u8DDRAM_Address = Local_u8DDRAM_Address + 128; /* set bit 7 (0b10000000) for Set-DDRAM-Address command */

	LCD_voidSendCommand(Local_u8DDRAM_Address);
}

/* Write a custom 5×8 character pattern stored in CGRAM, then display it.
 *   Copy_u8Pattern      – pointer to 8-byte array defining the character rows
 *   Copy_u8XPos / YPos  – position to display the character on screen
 *   Copy_u8PatternNumber– CGRAM slot (0–7; each slot = 8 bytes)
 *
 * Sequence:
 *   1. Compute CGRAM start address for the chosen slot (slot * 8).
 *   2. Send Set-CGRAM-Address command (bit 6 = 1, i.e. +64).
 *   3. Write 8 data bytes to define the character shape.
 *   4. Move cursor to display position and send the pattern index as data. */
void LCD_voidWriteSpecialChar(u8 *Copy_u8Pattern, u8 Copy_u8XPos, u8 Copy_u8YPos, u8 Copy_u8PatternNumber)
{
	u8 Local_u8CGRAM_Address = Copy_u8PatternNumber * 8; /* each CGRAM slot is 8 bytes */

	LCD_voidSendCommand(Local_u8CGRAM_Address + 64); /* 64 = 0b01000000 → Set-CGRAM-Address command */

	/* Write the 8-row bitmap of the custom character into CGRAM */
	for (u8 Local_u8Counter = 0; Local_u8Counter < 8; Local_u8Counter++)
	{
		LCD_voidSendData(Copy_u8Pattern[Local_u8Counter]);
	}

	/* Move to the target display position and render the character */
	LCD_voidGoToXY(Copy_u8XPos, Copy_u8YPos);
	LCD_voidSendData(Copy_u8PatternNumber); /* send slot index; LCD maps it to the CGRAM pattern */
}
