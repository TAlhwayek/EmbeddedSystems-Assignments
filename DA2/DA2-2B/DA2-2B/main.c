/*
 * DA2-2B.c
 *
 * Created: 3/4/2023 6:22:20 PM
 * Author : Tony Alhwayek
 */ 

#define F_CPU 16000000UL
#include <avr/io.h>
#include <avr/interrupt.h>
#include <util/delay.h>


int main(void)
{
	
	DDRB = 1<<5;				// Set PORTB5 as output
	PORTB = 0x3C;				// Turn off all 4 LEDs (they sometimes turn on by themselves)
	DDRD &= (0<<2);				// Set PIND2 as input
	PORTD = 1<<2;				// Enable pull-up resistor on PORTD.2				
	EICRA = 0x2;				// Make INT0 falling edge triggered
	EIMSK = (1<<INT0);			// Enable external interrupt 0
	sei();						// Enable global interrupts
	

    while (1);					// Needed while loop
}

	ISR(INT0_vect) {			// Interrupt 0
		PORTB &= ~(1 << PB5);	// Turn on the LED at PB5
		_delay_ms(3500);		// 3.5s delay
		PORTB |= (1 << PB5);	// Turn off the LED
		
	}



