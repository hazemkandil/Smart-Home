#ifndef BIT_MATH_H
#define BIT_MATH_H

#define Set_Bit(reg,bit) (reg|=(1<<bit))
#define Get_Bit(reg,bit) ((reg>>bit) & 1)
#define Clear_Bit(reg,bit) (reg&=~(1<<bit))
#define Toggle_Bit(reg,bit) (reg^=(1<<bit))


#endif