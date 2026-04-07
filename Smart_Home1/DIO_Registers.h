/*
 ************ Author : Hazem Mohamed ****************
 ************ Date : 30-8-2022 *******************
 */
#ifndef DIO_REGISTERS_H_
#define DIO_REGISTERS_H_

// port A

#define PORTA      *((volatile u8*)(0x3B))
#define DDRA       *((volatile u8*)(0x3A))
#define PINA       *((volatile u8*)(0x39))

// port B

#define PORTB      *((volatile u8*)(0x38))
#define DDRB       *((volatile u8*)(0x37))
#define PINB       *((volatile u8*)(0x36))

// port C

#define PORTC      *((volatile u8*)(0x35))
#define DDRC       *((volatile u8*)(0x34))
#define PINC       *((volatile u8*)(0x33))

// port D

#define PORTD      *((volatile u8*)(0x32))
#define DDRD       *((volatile u8*)(0x31))
#define PIND       *((volatile u8*)(0x30))


#endif /* DIO_REGISTERS_H_ */
