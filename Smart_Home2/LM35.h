/*
 * LM35.h
 *
 *  Created on: Sep 6, 2022
 *      Author: Hazem Mohamed
 */

#ifndef LM35_H_
#define LM35_H_

//void LM35_voidDisplayBits(ADC_CHANNELS_options_t Copy_u8ADCChannel);

void LM35_voidDisplayVolt(f32 Local_f32Volt);

f32 LM35_voidVoltToTemperature(f32 Copy_f32ADCVolt);

#endif /* LM35_H_ */
