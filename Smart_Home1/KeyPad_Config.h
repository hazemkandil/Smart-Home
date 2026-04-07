#ifndef KEYPAD_CONFIG_H
#define KEYPAD_CONFIG_H


#define KEYPAD_PORT			DIO_PORTA

#define KEYPAD_ROW1_PIN		PIN0
#define KEYPAD_ROW2_PIN		PIN1
#define KEYPAD_ROW3_PIN		PIN2
#define KEYPAD_ROW4_PIN		PIN3

#define KEYPAD_COL1_PIN		PIN4
#define KEYPAD_COL2_PIN		PIN5
#define KEYPAD_COL3_PIN		PIN6

const u8 Keypad_Char[16]={ 	'1', '2', '3',
							'4', '5', '6',
							'7', '8', '9',
							'*', '0', '#',	};


#endif
