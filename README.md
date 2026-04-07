# 🏠 Smart Home – Embedded Security & Automation System

> A two-MCU embedded Smart Home system built from scratch in bare-metal C for ATmega32 and ATmega16, featuring keypad authentication, servo-controlled door lock, temperature monitoring, and ambient light control. Simulated in Proteus.

---

## 📑 Table of Contents

- [Overview](#overview)
- [System Architecture](#system-architecture)
- [Hardware Components](#hardware-components)
- [Authentication Flow](#authentication-flow)
- [Sensor Automation](#sensor-automation)
- [Project Structure](#project-structure)
- [Drivers & Peripherals](#drivers--peripherals)
- [USART Communication Protocol](#usart-communication-protocol)
- [Build & Flash](#build--flash)
- [Simulation](#simulation)
- [Default Credentials](#default-credentials)
- [Author](#author)

---

## Overview

This project implements a Smart Home security and automation system split across two cooperating AVR microcontrollers that communicate over a USART serial link:

| Node | MCU | Role |
|---|---|---|
| **Smart_Home1** | ATmega32 | Keypad input & user feedback (Transmitter / Input MCU) |
| **Smart_Home2** | ATmega16 | Authentication logic, LCD display & home automation (Controller / Display MCU) |

The system supports:
- **Secure door entry** via 4-digit username + 4-digit password authentication
- **3-attempt lockout** with visual feedback on a 7-segment/LED display
- **Servo motor** driven door lock (Timer1 PWM)
- **Temperature monitoring** with automatic fan/AC control (LM35 sensor)
- **Ambient light monitoring** with proportional LED bar control (LDR sensor)

---

## System Architecture

```
┌─────────────────────────────────┐        USART (9600 bps, 8N1)        ┌──────────────────────────────────────┐
│         Smart_Home1              │ ◄──────────────────────────────────► │          Smart_Home2                  │
│         ATmega32                 │                                       │          ATmega16                     │
│                                  │                                       │                                       │
│  ┌──────────┐  ┌──────────────┐  │                                       │  ┌────────┐  ┌───────────────────┐   │
│  │  4×3     │  │  7-Segment / │  │                                       │  │  LCD   │  │  Servo Motor PWM  │   │
│  │ Keypad   │  │  LED Display │  │                                       │  │  16×2  │  │  (Timer1 OC1A)    │   │
│  └──────────┘  └──────────────┘  │                                       │  └────────┘  └───────────────────┘   │
│                                  │                                       │                                       │
│  ┌──────────────────────────┐    │                                       │  ┌────────┐  ┌────────┐              │
│  │ INT0 Door-Bell Button    │    │                                       │  │  LM35  │  │  LDR   │              │
│  │ (PD2, falling-edge ISR)  │    │                                       │  │  ADC1  │  │  ADC0  │              │
│  └──────────────────────────┘    │                                       │  └────────┘  └────────┘              │
└─────────────────────────────────┘                                       │                                       │
                                                                           │  ┌────────┐  ┌────────┐              │
                                                                           │  │  Fan/  │  │  8-LED │              │
                                                                           │  │  AC    │  │  Bar   │              │
                                                                           │  │ PA3    │  │ PORTB  │              │
                                                                           │  └────────┘  └────────┘              │
                                                                           └──────────────────────────────────────┘
```

---

## Hardware Components

### Smart_Home1 (ATmega32 – Input MCU)

| Component | Connection | Description |
|---|---|---|
| 4×3 Matrix Keypad | PORTC | Numeric digit input (0–9) |
| 7-Segment / LED Display | PORTB | Shows remaining login attempts (3→2→1→0) |
| Door-Bell Push Button | PD2 (INT0) | Falls-edge interrupt to trigger login flow |
| USART TX | PD1 | Serial data out to Smart_Home2 |
| USART RX | PD0 | Control token input from Smart_Home2 |

### Smart_Home2 (ATmega16 – Control MCU)

| Component | Connection | Description |
|---|---|---|
| HD44780 16×2 LCD | Configurable (LCD_Configure.h) | User interface display |
| Servo Motor | PD5 (OC1A, Timer1 PWM) | Physical door lock mechanism |
| LM35 Temperature Sensor | ADC1 (PA1) | Ambient temperature reading |
| LDR Light Sensor | ADC0 (PA0) | Ambient light level reading |
| Fan / AC Relay | PA3 | Activated when temperature ≥ 26 °C |
| 8-LED Light Bar | PORTB | Proportional to ambient light (0–8 LEDs) |
| USART RX | PD0 | Receives data from Smart_Home1 |
| USART TX | PD1 | Sends control tokens to Smart_Home1 |

---

## Authentication Flow

```
[User presses door-bell button on Smart_Home1]
          │
          ▼
[INT0 ISR fires → disables INT0 → sends '/' to Smart_Home2]
          │
          ▼
[Smart_Home2 receives '/']
          │
          ▼
[LCD: "Welcome to the Smart Home" (5 s)]
          │
          ▼
┌─────────────────────────────────────────────┐
│  Credential Entry Loop (up to 3 attempts)   │
│                                             │
│  Smart_Home2 → sends '-'/'W'/'N' token      │
│  Smart_Home1 → shows attempt count on LEDs  │
│                                             │
│  1. Prompt "Enter User:" on LCD             │
│     → Smart_Home1 keypad captures 4 digits  │
│     → each digit TX'd over USART            │
│     → displayed on LCD line 2               │
│                                             │
│  2. Prompt "Enter Password:" on LCD         │
│     → 4 digits, shown briefly then masked   │
│       with '*' (200 ms visibility)          │
│                                             │
│  3. Compare against DEFAULT_USER / PASS     │
│     ├── MATCH   → exit loop, open door ─────┼──►[Door Open Sequence]
│     └── MISMATCH                            │
│          ├── Attempt 1: send 'W' → retry   │
│          ├── Attempt 2: send 'N' → retry   │
│          └── Attempt 3: send 'T' then '.'  │
│               → LCD: "System Locked"       │
│               → Smart_Home1 re-ens. INT0   │
└─────────────────────────────────────────────┘

[Door Open Sequence]
  Smart_Home2 sends 'D' → Smart_Home1 blanks display
  LCD: "Opening Door..."
  Servo sweeps: OCR1A 751 → 1250 (1 ms/step, ~500 ms sweep)
  LCD: "Door Opened"
  → Enters Sensor Monitoring Loop (see below)
```

### USART Token Table

| Token | Direction | Meaning |
|---|---|---|
| `'/'` | Home1 → Home2 | Door-bell pressed, begin login |
| `'-'` | Home2 → Home1 | 1st login attempt — show "3" |
| `'W'` | Home2 → Home1 | 1st wrong attempt — show "2" |
| `'N'` | Home2 → Home1 | 2nd wrong attempt — show "1" |
| `'T'` | Home2 → Home1 | System locked — show "0" |
| `'.'` | Home2 → Home1 | Lock confirmation (re-enable INT0) |
| `'D'` | Home2 → Home1 | Success — blank display |

---

## Sensor Automation

Once the door is successfully opened, **Smart_Home2** enters a continuous sensor monitoring loop:

### 🌡️ Temperature Control (LM35 – ADC1)

- Reads ADC1 and converts voltage to °C using the LM35's **10 mV/°C** characteristic
- Displays `Temp: XX.X` on LCD line 1
- **≥ 26 °C** → turns **fan ON** (PA3 HIGH) and shows a custom fan icon on LCD col 15
- **< 26 °C** → turns **fan OFF** (PA3 LOW)

### 💡 Light Control (LDR – ADC0)

- Reads ADC0 and maps the voltage to 0–8 LEDs on PORTB proportionally:

| ADC Voltage | LEDs Lit |
|---|---|
| ≤ 0.30 V | 0 |
| 0.31 – 0.50 V | 1 |
| 0.51 – 0.90 V | 2 |
| 0.91 – 1.70 V | 3 |
| 1.71 – 2.60 V | 4 |
| 2.61 – 3.40 V | 5 |
| 3.41 – 4.20 V | 6 |
| 4.21 – 4.59 V | 7 |
| ≥ 4.60 V | 8 |

- Displays `LEDS: N` on LCD line 2

---

## Project Structure

```
Smart Home/
├── Source Code/
│   ├── Smart_Home1/          # ATmega32 – Keypad / Transmitter MCU
│   │   ├── main.c            # System entry point, interrupt setup, main loop
│   │   ├── APP_Program.c     # Keypad read & USART transmit logic
│   │   ├── APP_Interface.h   # Application-layer function declarations
│   │   ├── DIO_Program.c     # Digital I/O driver
│   │   ├── EXTI_Program.c    # External interrupt (INT0) driver
│   │   ├── USART_Program.c   # USART driver (9600 bps, 8N1)
│   │   ├── Timer1_Program.c  # Timer1 driver
│   │   ├── ADC_Program.c     # ADC driver
│   │   ├── KeyPad_Program.c  # 4×3 matrix keypad driver
│   │   ├── LCD_Program.c     # HD44780 LCD driver
│   │   ├── Std_Types.h       # Portable type aliases (u8, u16, f32 …)
│   │   ├── Bit_Math.h        # Bit manipulation macros
│   │   └── *_Registers.h     # Peripheral register maps
│   │
│   ├── Smart_Home2/          # ATmega16 – Display / Control MCU
│   │   ├── main.c            # Authentication, door control, sensor monitoring
│   │   ├── APP_Program.c     # Credential display helpers & comparison
│   │   ├── APP_Interface.h   # Credential config (DEFAULT_USER, DEFAULT_PASS …)
│   │   ├── LM35.c / LM35.h   # LM35 temperature sensor driver
│   │   ├── DIO_Program.c     # Digital I/O driver
│   │   ├── EXTI_Program.c    # External interrupt driver
│   │   ├── USART_Program.c   # USART driver
│   │   ├── Timer1_Program.c  # Timer1 PWM driver (servo control)
│   │   ├── ADC_Program.c     # ADC driver (LM35 + LDR)
│   │   ├── KeyPad_Program.c  # Keypad driver
│   │   ├── LCD_Program.c     # HD44780 LCD driver
│   │   ├── Std_Types.h       # Portable type aliases
│   │   ├── Bit_Math.h        # Bit manipulation macros
│   │   └── *_Registers.h     # Peripheral register maps
│   │
│   └── Proteus Simulation/
│       ├── Smart_Home.pdsprj # Proteus project file
│       └── Smart_Home.PDF    # Circuit schematic PDF export
│
└── README.md
```

---

## Drivers & Peripherals

All drivers follow a **layered architecture** with three consistent files:

| File pattern | Purpose |
|---|---|
| `XXX_Registers.h` | Raw register address definitions mapped to hardware |
| `XXX_Interface.h` | Public API – function prototypes & configuration macros |
| `XXX_Program.c` | Driver implementation |

### Driver Summary

| Driver | Peripheral | Key API |
|---|---|---|
| **DIO** | GPIO (PORTA–PORTD) | `DIO_voidSetPinDirection`, `DIO_voidSetPinValue`, `DIO_u8GetPinValue`, `DIO_voidPullUpPin` |
| **USART** | Hardware UART | `USART_voidInit`, `USART_voidWriteData`, `USART_u8ReadData` |
| **ADC** | 10-bit SAR ADC | `ADC_voidInit`, `ADC_u16Read`, `ADC_f32ADCtoVolt` |
| **Timer1** | 16-bit Timer / PWM | `PWM_voidInitTimer1`, `PWM_voidSetCompareMatchValue` |
| **EXTI** | External Interrupts | `EXTI_voidGlobalEnable`, `EXTI_voidINT0Init`, `EXTI_voidINT0End`, `EXTI_voidCallBackINT0` |
| **LCD** | HD44780 16×2 | `LCD_voidInit`, `LCD_voidWriteString`, `LCD_voidGoToXY`, `LCD_voidWriteSpecialChar` |
| **KeyPad** | 4×3 Matrix Keypad | `KeyPad_voidInit`, `KeyPad_u8GetPress` |
| **LM35** | Temperature Sensor | `LM35_voidVoltToTemperature`, `LM35_voidDisplayVolt` |

---

## USART Communication Protocol

Both MCUs communicate at **9600 bps, 8N1** (8 data bits, no parity, 1 stop bit) over a direct USART serial link:

- **Smart_Home1 TX (PD1)** → **Smart_Home2 RX (PD0)**
- **Smart_Home2 TX (PD1)** → **Smart_Home1 RX (PD0)**

All protocol tokens are single ASCII bytes. A 30 ms inter-character delay on the transmit side ensures the receiving MCU's blocking read completes before the next byte is sent.

---

## Build & Flash

### Prerequisites

- **WinAVR** (avr-gcc toolchain) or **Atmel Studio / Microchip Studio**
- **AVRDUDE** for flashing (if using physical hardware)
- **Proteus** 8+ for simulation

### Compiling (WinAVR / command line)

```bash
# Smart_Home1 (ATmega32, 16 MHz)
avr-gcc -mmcu=atmega32 -DF_CPU=16000000UL -O1 -Wall \
        *.c -o Smart_Home1.elf
avr-objcopy -O ihex Smart_Home1.elf Smart_Home1.hex

# Smart_Home2 (ATmega16, 16 MHz)
avr-gcc -mmcu=atmega16 -DF_CPU=16000000UL -O1 -Wall \
        *.c -o Smart_Home2.elf
avr-objcopy -O ihex Smart_Home2.elf Smart_Home2.hex
```

### Flashing (AVRDUDE example)

```bash
# ATmega32
avrdude -c usbasp -p m32 -U flash:w:Smart_Home1.hex

# ATmega16
avrdude -c usbasp -p m16 -U flash:w:Smart_Home2.hex
```

> Both projects can also be opened directly in **Eclipse / Atmel Studio** using the included `.cproject` / `.project` files.

---

## Simulation

The full circuit is provided as a **Proteus 8** project:

```
Proteus Simulation/
├── Smart_Home.pdsprj    ← Open this in Proteus
└── Smart_Home.PDF       ← Schematic PDF (for viewing without Proteus)
```

Load the compiled `.hex` files for each MCU in the Proteus component properties before running the simulation.

---

## Default Credentials

| Field | Value |
|---|---|
| Username | `1234` |
| Password | `5678` |

To change the defaults, edit `APP_Interface.h` inside `Smart_Home2/`:

```c
#define DEFAULT_USER "1234"   /* 4-digit username */
#define DEFAULT_PASS "5678"   /* 4-digit password */
```

> ⚠️ Credentials are stored in flash as plain text. This project is intended for educational / simulation purposes.

---

## Author

**Hazem Mohamed**  
Embedded Systems Engineer  
Project created: September 2022

---

*Built with bare-metal C on AVR | Simulated in Proteus | No external libraries or RTOS*
