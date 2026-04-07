#ifndef KEYPAD_INTERFACE_H
#define KEYPAD_INTERFACE_H


#define KEYPAD_NOT_PRESSED 255


void KeyPad_voidInit(void);
u8 KeyPad_u8ReadButton(u8 Copy_u8Line);
void KeyPad_voidSelectLine(u8 Copy_u8Line);
u8 KeyPad_u8GetPress(void);

#endif
