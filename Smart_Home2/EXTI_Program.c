/*
 * EXTI_Program.c
 *
 *  Created on: Sep 5, 2022
 *      Author: Hazem Mohamed
 *
 *  Description:
 *      External Interrupt (EXTI) driver for the ATmega16 (Smart_Home2).
 *      Provides initialisation for INT0, INT1, INT2 and global
 *      interrupt enable/disable helpers.  Smart_Home2 does not use
 *      the interrupt-driven door signal; these functions are provided
 *      for completeness and future use.
 */

#include "Std_Types.h"
#include "Bit_Math.h"

#include "EXTI_Registers.h"
#include "EXTI_Interface.h"

/* ------------------------------------------------------------------ */
/* INT0 – falling-edge trigger (ISC01=1, ISC00=0).                    */
/* ------------------------------------------------------------------ */
void EXTI_voidINT0Init(void)
{
	/* ISC01=1, ISC00=0 → falling-edge sense */
	Clear_Bit(MCUCR, MCUCR_ISC00);
	Set_Bit  (MCUCR, MCUCR_ISC01);

	/* Enable INT0 in the General Interrupt Control Register */
	Set_Bit(GICR, GICR_INT0);
}

/* ------------------------------------------------------------------ */
/* INT1 – falling-edge trigger (ISC11=1, ISC10=0).                    */
/* ------------------------------------------------------------------ */
void EXTI_voidINT1Init(void)
{
	/* ISC11=1, ISC10=0 → falling-edge sense */
	Clear_Bit(MCUCR, MCUCR_ISC10);
	Set_Bit  (MCUCR, MCUCR_ISC11);

	Set_Bit(GICR, GICR_INT1);
}

/* ------------------------------------------------------------------ */
/* INT2 – falling-edge trigger (ISC2=0 in MCUCSR).                    */
/* ------------------------------------------------------------------ */
void EXTI_voidINT2Init(void)
{
	/* ISC2=0 → falling-edge sense */
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
