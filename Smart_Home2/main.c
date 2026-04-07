/*
 * main.c  –  Smart_Home2 (ATmega16 – Display / Control MCU)
 *
 *  Created on: Sep 19, 2022
 *      Author: Hazem Mohamed
 *
 * -----------------------------------------------------------------------
 *  SYSTEM OVERVIEW
 * -----------------------------------------------------------------------
 *  Smart_Home2 is the display and control node of a two-MCU Smart Home system.
 *  It is connected to Smart_Home1 (ATmega32) via a USART serial link.
 *
 *  Responsibilities:
 *    1. Display status messages on a 16×2 HD44780 LCD.
 *    2. Receive credential digits from Smart_Home1 and compare them
 *       against the stored DEFAULT_USER and DEFAULT_PASS values.
 *    3. Send control tokens back to Smart_Home1:
 *         '-'  → prompt for 1st attempt
 *         'W'  → 1st wrong attempt warning
 *         'N'  → 2nd wrong attempt warning
 *         'T'  → system locked (3rd failure), followed by '.'
 *         'D'  → credentials accepted, open door
 *    4. On successful login, sweep the servo motor (via Timer1 PWM on OC1A)
 *       to open the door (OCR1A: 751 → 1250, 1 ms/step).
 *    5. After door opens, continuously monitor:
 *         - LM35 temperature sensor (ADC1): display in °C; activate fan
 *           (PORTA PIN3) when temperature ≥ 26 °C; show fan icon on LCD.
 *         - LDR light sensor (ADC0): control 8 LEDs on PORTB proportionally
 *           to ambient light level.
 *
 *  Hardware connections (ATmega16):
 *    PORTD PIN0       – USART RX (input, from Smart_Home1 TX)
 *    PORTD PIN1       – USART TX (output, to Smart_Home1 RX)
 *    PORTD PIN5       – OC1A PWM output → servo motor signal
 *    PORTA PIN3       – Fan / AC relay output
 *    PORTB (all pins) – LED bar (8 LEDs, controlled by light sensor)
 *    ADC0 (PORTA PIN0)– LDR (light-dependent resistor) input
 *    ADC1 (PORTA PIN1)– LM35 temperature sensor input
 *    LCD data port    – configured in LCD_Configure.h
 *
 *  Authentication flow:
 *    Idle → wait for '/' (door-bell pressed on Smart_Home1)
 *         → show "Welcome to the Smart Home" (5 s)
 *         → loop: prompt "Enter User:" → receive 4 digits
 *                 prompt "Enter Password:" → receive 4 digits
 *                 compare → wrong → warn / lock
 *                         → correct → open door → monitor sensors
 * -----------------------------------------------------------------------
 */

#include "Bit_Math.h"
#include "Std_Types.h"

#include "DIO_Interface.h"
#include "LM35.h"
#include "ADC_Interface.h"
#include "USART_Interface.h"
#include "Timer1_Interface.h"
#include "LCD_Interface.h"
#include "LCD_Configure.h"

#include "util/delay.h"

#include "APP_Interface.h"

/* Custom character bitmap for the fan/motor icon (8 rows × 5 cols).
 * Stored in CGRAM slot PATTERN_0 and shown at LCD column 15, line 0
 * when the fan is active (temperature ≥ 26 °C). */
u8 Global_u8Motor[8] = {17, 27, 27, 21, 17, 17, 17, 17};

int main(void)
{
	/* ---- Peripheral Initialisation ---- */
	ADC_voidInit();              /* configure ADC: AREF, left-adj off, div/64  */
	LCD_voidConfigDirection();   /* set LCD data port and control pins as output */
	LCD_voidInit();              /* power-on initialisation sequence             */

	USART_voidInit();            /* 9600 bps, 8N1, TX+RX enabled                */

	/* USART pins: PD0 = RX (input), PD1 = TX (output) */
	DIO_voidSetPinDirection(DIO_PORTD, PIN0, INPUT_PIN);
	DIO_voidSetPinDirection(DIO_PORTD, PIN1, OUTPUT_PIN);

	/* PD5 = OC1A output for Timer1 PWM (servo motor) */
	DIO_voidSetPinDirection(DIO_PORTD, PIN5, OUTPUT_PIN);

	/* Repeat pin configuration (overrides above; kept as original) */
	DIO_voidSetPinDirection(DIO_PORTD, PIN0, INPUT_PIN);
	DIO_voidSetPinDirection(DIO_PORTD, PIN1, INPUT_PIN);

	/* PORTA PIN3 = fan/AC relay output */
	DIO_voidSetPinDirection(DIO_PORTA, PIN3, OUTPUT_PIN);

	/* PORTB = 8-LED light-level bar (all output) */
	DIO_voidSetPortDirection(DIO_PORTB, OUTPUT_PORT);

	/* ---- Runtime Variables ---- */
	u8 Local_u8Read;

	/* Stored credentials (compared against user input each attempt) */
	u8 Local_ArrUser[CREDENTIAL_BUFF_SIZE] = {DEFAULT_USER};
	u8 Local_ArrPass[CREDENTIAL_BUFF_SIZE] = {DEFAULT_PASS};

	/* Buffers for incoming digits from Smart_Home1 */
	u8 Local_ArrUserIN[CREDENTIAL_BUFF_SIZE];
	u8 Local_ArrPassIN[CREDENTIAL_BUFF_SIZE];

	/* Login flow control flags (see APP_Interface.h for macro definitions) */
	u8 Local_u8LoginPending   = FLAG_SET;   /* 1 = keep prompting for credentials */
	u8 Local_u8FailedAttempts = FLAG_CLEAR; /* counts wrong login attempts (0-2)  */

	/* ---- Main Loop ---- */
	while (1)
	{
		/* Reset per-cycle login-success flag:
		 *   FLAG_SET   = door-open path (default assumption)
		 *   FLAG_CLEAR = system-locked path (set if 3rd failure occurs) */
		u8 Local_u8LoginSuccess = FLAG_SET;

		/* Block until Smart_Home1 sends the '/' door-bell token */
		Local_u8Read = USART_u8ReadData();

		if (Local_u8Read == '/')
		{
			/* ---- Welcome Screen (shown once per door-bell press) ---- */
			LCD_voidSendCommand(0b00000001); /* clear display              */
			_delay_ms(20);
			LCD_voidWriteString("Welcome to the");
			LCD_voidGoToXY(3, 1);
			LCD_voidWriteString("Smart Home");
			_delay_ms(5000);                 /* hold welcome for 5 seconds */
			LCD_voidSendCommand(0b00000001); /* clear display              */
			_delay_ms(200);

			/* ---- Credential Entry Loop ---- */
			do {
				LCD_voidSendCommand(0b00000001); /* clear display before each attempt */
				LCD_voidWriteString("Enter User:");

				/* Signal Smart_Home1 to start collecting username digits
				 * only on the first attempt; subsequent loops skip this
				 * to avoid resetting Smart_Home1's attempt counter. */
				if (Local_u8FailedAttempts == FLAG_CLEAR)
					USART_voidWriteData('-');

				/* Receive CREDENTIAL_LENGTH (4) username digits from Smart_Home1 */
				for (u8 Index = 0; Index < CREDENTIAL_LENGTH; Index++)
				{
					u8 Read = USART_u8ReadData();
					APP_voidReceiveUser(Read);          /* show on LCD line 2 */
					Local_ArrUserIN[Index] = Read;      /* store for comparison */
				}
				_delay_ms(1000); /* brief pause before showing password prompt */

				LCD_voidSendCommand(1);
				LCD_voidWriteString("Enter Password:");

				/* Receive CREDENTIAL_LENGTH (4) password digits from Smart_Home1 */
				for (u8 Index = 0; Index < CREDENTIAL_LENGTH; Index++)
				{
					u8 Read = USART_u8ReadData();
					APP_voidReceivePassword(Read);      /* show masked on LCD  */
					Local_ArrPassIN[Index] = Read;
				}
				_delay_ms(1000);

				/* ---- Credential Validation ---- */
				if (!APP_u8UserCheck(Local_ArrPassIN, Local_ArrPass) ||
				    !APP_u8UserCheck(Local_ArrUserIN, Local_ArrUser))
				{
					/* Wrong credentials */
					if (Local_u8FailedAttempts == MAX_LOGIN_ATTEMPTS)
					{
						/* 3rd failure → lock the system */
						USART_voidWriteData('T');        /* notify Smart_Home1: locked     */
						LCD_voidSendCommand(1);
						LCD_voidWriteString("System Locked");
						_delay_ms(250);
						USART_voidWriteData('.');        /* send lock-confirmation byte    */
						LCD_voidSendCommand(1);
						Local_u8FailedAttempts = FLAG_CLEAR;  /* reset for next door-bell */
						Local_u8LoginSuccess   = FLAG_CLEAR;  /* skip door-open path      */
						_delay_ms(1000);
						break;                           /* exit credential loop           */
					}
					else
					{
						/* 1st or 2nd failure → warn and try again */
						LCD_voidSendCommand(1);
						LCD_voidWriteString("Wrong Data");
						LCD_voidGoToXY(0, 1);
						LCD_voidWriteString("Try Again");

						if (Local_u8FailedAttempts == FLAG_CLEAR) {
							USART_voidWriteData('W');   /* 1st wrong attempt  */
							Local_u8FailedAttempts++;
						}
						else if (Local_u8FailedAttempts == 1) {
							USART_voidWriteData('N');   /* 2nd wrong attempt  */
							Local_u8FailedAttempts++;
						}
						_delay_ms(1000);
					}
				}
				else
				{
					/* Correct credentials → exit the retry loop */
					Local_u8LoginPending = FLAG_CLEAR;
				}

			} while (Local_u8LoginPending);

			/* ---- Post-Login Path ---- */
			if (Local_u8LoginSuccess == FLAG_CLEAR)
				continue; /* locked → go back and wait for next door-bell '/' */
			else
			{
				/* ---- Door Open Sequence ---- */
				USART_voidWriteData('D');            /* notify Smart_Home1: success    */
				LCD_voidSendCommand(1);
				LCD_voidWriteString("Opening Door...");
				_delay_ms(200);

				/* Initialise Timer1 PWM for servo control (50 Hz, Mode 14) */
				PWM_voidInitTimer1();

				/* Sweep servo from closed (751) to open (1250): 1 tick per ms
				 * Total sweep time ≈ 500 ms for smooth door rotation */
				for (u16 Local_u16Index = 751; Local_u16Index <= 1250; Local_u16Index++)
				{
					PWM_voidSetCompareMatchValue(Local_u16Index);
					_delay_ms(1);
				}

				LCD_voidSendCommand(1);
				LCD_voidWriteString("Door Opened");
				_delay_ms(250);
				LCD_voidSendCommand(1);

				/* ---- Sensor Monitoring Loop (runs indefinitely after door opens) ---- */
				while (1)
				{
					u8 LEDS; /* number of LEDs currently lit (0–8) */

					/* -- Temperature Monitoring (LM35 on ADC1) -- */
					u16 Local_u16ADC1Val  = ADC_u16Read(ADC1);
					f32 Local_f32ADC1Val  = ADC_f32ADCtoVolt(Local_u16ADC1Val);

					LCD_voidGoToXY(0, 0);
					LCD_voidWriteString("Temp: ");

					/* Convert voltage to °C (LM35: 10 mV/°C → V × 100) and display */
					f32 TemP = LM35_voidVoltToTemperature(Local_f32ADC1Val);

					if (TemP >= 26.00)
					{
						/* Temperature at or above threshold → fan ON */
						DIO_voidSetPinValue(DIO_PORTA, PIN3, HIGH);
						/* Display rotating fan icon in CGRAM slot PATTERN_0 at col 15 */
						LCD_voidWriteSpecialChar(Global_u8Motor, 15, 0, PATTERN_0);
					}
					else
					{
						/* Below threshold → fan OFF, show blank space at col 15 */
						DIO_voidSetPinValue(DIO_PORTA, PIN3, LOW);
						LCD_voidGoToXY(15, 0);
						LCD_voidSendData(0b00010000); /* right-arrow character (placeholder) */
					}

					/* -- Light Monitoring (LDR on ADC0) -- */
					/* ADC0 voltage → number of LEDs to illuminate (0–8 levels) */
					u16 Local_u16ADC0Val = ADC_u16Read(ADC0);
					f32 Local_f32ADC0Val = ADC_f32ADCtoVolt(Local_u16ADC0Val);

					LCD_voidGoToXY(0, 1);
					LCD_voidWriteString("LEDS: ");

					/* Map ADC voltage range to LED bitmask and count:
					 *   ≥ 4.6 V  → 8 LEDs (very bright)
					 *   ≤ 0.3 V  → 0 LEDs (very dark)
					 *   Ranges in between → proportional LED count */
					if (Local_f32ADC0Val >= 4.6) {
						DIO_voidSetPortValue(DIO_PORTB, 0B11111111); LEDS = 8;
					} else if (Local_f32ADC0Val <= 0.3) {
						DIO_voidSetPortValue(DIO_PORTB, 0B0);        LEDS = 0;
					} else if (Local_f32ADC0Val >= 0.31 && Local_f32ADC0Val <= 0.5) {
						DIO_voidSetPortValue(DIO_PORTB, 0B00000001); LEDS = 1;
					} else if (Local_f32ADC0Val >= 0.51 && Local_f32ADC0Val <= 0.9) {
						DIO_voidSetPortValue(DIO_PORTB, 0B00000011); LEDS = 2;
					} else if (Local_f32ADC0Val >= 0.91 && Local_f32ADC0Val <= 1.7) {
						DIO_voidSetPortValue(DIO_PORTB, 0B00000111); LEDS = 3;
					} else if (Local_f32ADC0Val >= 1.71 && Local_f32ADC0Val <= 2.6) {
						DIO_voidSetPortValue(DIO_PORTB, 0B00001111); LEDS = 4;
					} else if (Local_f32ADC0Val >= 2.61 && Local_f32ADC0Val <= 3.4) {
						DIO_voidSetPortValue(DIO_PORTB, 0B00011111); LEDS = 5;
					} else if (Local_f32ADC0Val >= 3.41 && Local_f32ADC0Val <= 4.2) {
						DIO_voidSetPortValue(DIO_PORTB, 0B00111111); LEDS = 6;
					} else if (Local_f32ADC0Val >= 4.21 && Local_f32ADC0Val < 4.6) {
						DIO_voidSetPortValue(DIO_PORTB, 0B01111111); LEDS = 7;
					}

					LCD_voidWriteNumber(LEDS); /* show LED count on LCD line 2 */
				}
			}
		}
	}

	return 0;
}
