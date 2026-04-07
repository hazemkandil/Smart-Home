/*
 * EXTI_Interface.h
 *
 *  Created on: Sep 5, 2022
 *      Author: Hazem Mohamed
 */

#ifndef EXTI_INTERFACE_H_
#define EXTI_INTERFACE_H_

void EXTI_voidINT0Init(void);

void EXTI_voidINT1Init(void);

void EXTI_voidINT2Init(void);

void EXTI_voidCallBackINT0(void(*Copy_pv0function)(void));

void EXTI_voidGlobalEnable(void);

void EXTI_voidGlobalDisable(void);

void EXTI_voidINT0End(void);

#endif /* EXTI_INTERFACE_H_ */
