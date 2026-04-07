/*
 * ADC_Registers.h
 *
 *  Created on: Sep 6, 2022
 *      Author: Hazem Mohamed
 */

#ifndef ADC_REGISTERS_H_
#define ADC_REGISTERS_H_

#define ADMUX      *((volatile u8*)0x27)

#define ADCSRA     *((volatile u8*)0x26)

#define ADCSRA_ADIE     3
#define ADCSRA_ADIF     4
#define ADCSRA_ADATE    5
#define ADCSRA_ADSC     6
#define ADCSRA_ADEN     7

#define ADCH       *((volatile u8*)0x25)
#define ADCLH      *((volatile u16*)0x24)

#define SFIOR      *((volatile u8*)0x50)

#endif /* ADC_REGISTERS_H_ */
