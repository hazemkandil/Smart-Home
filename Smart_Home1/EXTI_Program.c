/*
 * EXTI_Program.c
 *
 *  Created on: Sep 5, 2022
 *      Author: Hazem Mohamed
 *
 *  Description:
 *      External Interrupt (EXTI) driver for the ATmega32.
 *      Configures INT0, INT1, and INT2 sense-control bits and enables
 *      the corresponding interrupt lines. Also provides global
 *      interrupt enable/disable and a callback registration for INT0,
 *      which is used by Smart_Home1 to detect the door-open signal
 *      from the keypad MCU (Smart_Home2).
 */

#include "Std_Types.h"
#include "Bit_Math.h"

#include "EXTI_Registers.h"
#include "EXTI_Interface.h"

/* Pointer to the user-registered callback function for INT0.
 * Initialized to NULL; must be set via EXTI_voidCallBackINT0()
 * before interrupts are enabled. */
void (*EXTI_pvCallBackFuncINT0) (void) = NULL;

/* ------------------------------------------------------------------ */
/* INT0 – used to detect the '/' unlock signal sent by Smart_Home2.   */
/* Configured for falling-edge trigger (ISC01=1, ISC00=0).            */
/* ------------------------------------------------------------------ */
void EXTI_voidINT0Init(void)
{
	/* ISC01=1, ISC00=0 → falling-edge sense */
	Clear_Bit(MCUCR, MCUCR_ISC00);
	Set_Bit  (MCUCR, MCUCR_ISC01);

	/* Enable INT0 in the General Interrupt Control Register */
	Set_Bit(GICR, GICR_INT0);
}

/* Disable INT0 after the callback fires so consecutive triggers are  */
/* ignored while the door sequence is running.                         */
void EXTI_voidINT0End(void)
{
	Clear_Bit(GICR, GICR_INT0);
}

/* ------------------------------------------------------------------ */
/* INT1 – available for future expansion; falling-edge trigger.        */
/* ------------------------------------------------------------------ */
void EXTI_voidINT1Init(void)
{
	/* ISC11=1, ISC10=0 → falling-edge sense */
	Clear_Bit(MCUCR, MCUCR_ISC10);
	Set_Bit  (MCUCR, MCUCR_ISC11);

	Set_Bit(GICR, GICR_INT1);
}

/* ------------------------------------------------------------------ */
/* INT2 – available for future expansion; falling-edge trigger.        */
/* ------------------------------------------------------------------ */
void EXTI_voidINT2Init(void)
{
	/* ISC2=0 → falling-edge sense (INT2 sense control is in MCUCSR) */
	Clear_Bit(MCUCSR, MCUCSR_ISC2);
	Set_Bit  (GICR,   GICR_INT2);
}

/* Enable the global interrupt flag (I-bit in SREG) */
void EXTI_voidGlobalEnable(void)
{
	/*Enable Global Interrupt*/
	Set_Bit(SREG, SREG_I);
}

/* Disable the global interrupt flag – halts all maskable interrupts */
void EXTI_voidGlobalDisable(void)
{
	/*Disable Global Interrupt*/
	Clear_Bit(SREG, SREG_I);
}

/* Register the function to call when INT0 fires.
 * In Smart_Home1 this is the Enable() function, which disables INT0
 * and sends '/' back to Smart_Home2 to begin the door-open sequence. */
void EXTI_voidCallBackINT0(void(*Copy_pv0function)(void))
{
    EXTI_pvCallBackFuncINT0 = Copy_pv0function;
}

/* INT0 ISR – __vector_1 is the AVR vector for External Interrupt 0.
 * Calls the registered callback, guarding against a NULL pointer.    */
void __vector_1(void) __attribute__((signal));
void __vector_1(void)
{
    if(EXTI_pvCallBackFuncINT0 != NULL)
    {
        EXTI_pvCallBackFuncINT0();
    }
}
