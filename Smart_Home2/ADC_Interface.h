/*
 * ADC_Interface.h
 *
 *  Created on: Sep 6, 2022
 *      Author: Hazem Mohamed
 */

#ifndef ADC_INTERFACE_H_
#define ADC_INTERFACE_H_

typedef enum{
	AREF,
	AVCC,
	RES,
	INTERNAL_2_56
}ADC_ref_options_t;

typedef enum{
	ADCLEFT_ADJUSTMENT_OFF,
	ADCLEFT_ADJUSTMENT_ON,
}ADC_Left_Adjustment_options_t;

typedef enum{
	ADC0,
	ADC1,
	ADC2,
	ADC3,
	ADC4,
	ADC5,
	ADC6,
	ADC7
}ADC_CHANNELS_options_t;

typedef enum
{
	DIV_2=1,
	DIV_4,
	DIV_8,
	DIV_16,
	DIV_32,
	DIV_64,
	DIV_128,
}Prescalar_VAL;

#define VREF    5

void ADC_voidInit(void);

u16 ADC_u16Read(ADC_CHANNELS_options_t Copy_u8ADCChannel);

f32 ADC_f32ADCtoVolt(u16 Copy_u16ADCVal);


#endif /* ADC_INTERFACE_H_ */
