/*
 * LCD_Program.c
 *
 *  Created on: Aug 31, 2022
 *      Author: Hazem Mohamed
 *
 *  Description:
 *      HD44780-compatible 16×2 LCD driver operating in 8-bit parallel mode.
 *      Used by Smart_Home2 to display "Welcome to the Smart Home",
 *      "Enter User:", "Enter Password:", "Wrong Data / Try Again",
 *      "System Locked", "Opening Door...", "Door Opened", and live
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
 * E is pulsed HIGH for 2 ms then LOW to latch the character. */
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
	/* Wait for LCD controller to stabilise after power-on (≥15 ms typical) */
	_delay_ms(35);

	/* Function Set: 8-bit interface, 2 display lines, 5×8 dot font */
	LCD_voidSendCommand(0b00111100);

	/* Display ON, cursor OFF, blink OFF */
	LCD_voidSendCommand(0b00001100);

	/* Clear Display (resets DDRAM address counter to 0) */
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
 * Used to display the LED count and temperature digits on the home screen. */
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
 *   Line 1 DDRAM: 0x00 – 0x0F
 *   Line 2 DDRAM: 0x40 – 0x4F
 * Set-DDRAM-Address command sets bit 7 of the address byte. */
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

	Local_u8DDRAM_Address = Local_u8DDRAM_Address + 128; /* set bit 7 → Set-DDRAM-Address command */

	LCD_voidSendCommand(Local_u8DDRAM_Address);
}

/* Write a custom 5×8 character pattern into CGRAM then display it.
 *   Copy_u8Pattern      – 8-byte bitmap (one byte per row of the character)
 *   Copy_u8XPos / YPos  – display position on screen
 *   Copy_u8PatternNumber– CGRAM slot index (0–7)
 *
 * In Smart_Home2 this is used to display the rotating fan icon (Global_u8Motor)
 * at column 15, line 0 when the fan/AC is active (temperature ≥ 26 °C). */
void LCD_voidWriteSpecialChar(u8 *Copy_u8Pattern, u8 Copy_u8XPos, u8 Copy_u8YPos, u8 Copy_u8PatternNumber)
{
	u8 Local_u8CGRAM_Address = Copy_u8PatternNumber * 8; /* each CGRAM slot = 8 bytes */

	LCD_voidSendCommand(Local_u8CGRAM_Address + 64); /* 64 = 0b01000000 → Set-CGRAM-Address command */

	/* Write the 8-row bitmap into CGRAM */
	for (u8 Local_u8Counter = 0; Local_u8Counter < 8; Local_u8Counter++)
	{
		LCD_voidSendData(Copy_u8Pattern[Local_u8Counter]);
	}

	/* Move to the display position and render the custom character */
	LCD_voidGoToXY(Copy_u8XPos, Copy_u8YPos);
	LCD_voidSendData(Copy_u8PatternNumber); /* slot index used by LCD to look up CGRAM bitmap */
}
