
#include "STD_TYPES.h"
#include "BIT_MATH.h"

#include "DIO_Interface.h"

#include "KeyPad_Interface.h"
#include "KeyPad_config.h"

#include <util/delay.h>

/*
 * KeyPad_Program.c  –  Smart_Home2 (ATmega16 – display/control MCU)
 *
 *  Created on: Sep 2022
 *      Author: Hazem Mohamed
 *
 *  Description:
 *      Driver for a 4×3 matrix keypad (identical logic to Smart_Home1).
 *      Scanning strategy: active-low row drive.
 *        1. Rows are outputs, initialised HIGH (inactive).
 *        2. Columns are inputs with pull-ups (HIGH when no key pressed).
 *        3. Each row is pulled LOW in turn (KeyPad_voidSelectLine).
 *        4. Columns are read; a LOW column means that key is pressed.
 *        5. The (row, column) index maps into Keypad_Char[] to get the
 *           ASCII character for credential entry.
 */

/* Pin arrays built from the config header so the port mapping is
 * defined in one single place (KeyPad_Config.h). */
u8 Rows[4] = {KEYPAD_ROW1_PIN, KEYPAD_ROW2_PIN, KEYPAD_ROW3_PIN, KEYPAD_ROW4_PIN};
u8 Cols[3] = {KEYPAD_COL1_PIN, KEYPAD_COL2_PIN, KEYPAD_COL3_PIN};

/* Configure all row pins as output-HIGH and all column pins as
 * input with internal pull-up. */
void KeyPad_voidInit(void){

	for(u8 rowCounter = 0; rowCounter < 4; rowCounter++){
		DIO_voidSetPinDirection(KEYPAD_PORT, Rows[rowCounter], OUTPUT_PIN);
		DIO_voidSetPinValue    (KEYPAD_PORT, Rows[rowCounter], HIGH);      /* idle = HIGH */
	}

	for(u8 colCounter = 0; colCounter < 3; colCounter++){
		DIO_voidSetPinDirection(KEYPAD_PORT, Cols[colCounter], INPUT_PIN);
		DIO_voidSetPinValue    (KEYPAD_PORT, Cols[colCounter], HIGH);      /* enable pull-up */
	}
}

/* Scan all rows and return the ASCII character of the pressed key.
 * A 2 ms delay after each row prevents column-read glitches. */
u8 KeyPad_u8GetPress(void){
	u8 Local_u8Col;
	u8 x = 0;  /* default index → first entry of Keypad_Char */

	for(u8 rowCounter = 0; rowCounter < 4; rowCounter++){
		KeyPad_voidSelectLine(Rows[rowCounter]);              /* drive row LOW  */
		Local_u8Col = KeyPad_u8ReadButton(Rows[rowCounter]);  /* scan columns   */
		if(Local_u8Col != KEYPAD_NOT_PRESSED)
			x = (rowCounter * 3) + Local_u8Col; /* flatten 2-D index to 1-D */
		_delay_ms(2);                           /* debounce / settle time   */
	}
	return Keypad_Char[x];
}

/* Scan all columns while the given row is driven LOW.
 * Returns the column index (0-based) of the first LOW column,
 * or KEYPAD_NOT_PRESSED if none found.
 * Restores the row to HIGH before returning. */
u8 KeyPad_u8ReadButton(u8 Copy_u8Line){

	for(u8 colCounter = 0; colCounter < 4; colCounter++){
		if(DIO_u8GetPinValue(KEYPAD_PORT, Cols[colCounter]) == LOW){
			DIO_voidSetPinValue(KEYPAD_PORT, Copy_u8Line, HIGH); /* deactivate row */
			return colCounter;
		}
	}

	DIO_voidSetPinValue(KEYPAD_PORT, Copy_u8Line, HIGH); /* deactivate row */
	return KEYPAD_NOT_PRESSED;
}

/* Drive the specified row LOW to select (activate) that row for scanning. */
void KeyPad_voidSelectLine(u8 Copy_u8Line){
	DIO_voidSetPinValue(KEYPAD_PORT, Copy_u8Line, LOW);
}
