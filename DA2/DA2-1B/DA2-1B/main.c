/*
 * DA2-1B.c
 *
 * Created: 3/1/2023 8:38:02 PM
 * Author : Tony Alhwayek
 */ 

#include <avr/io.h>
#define F_CPU 16000000UL  
#include <util/delay.h>
int main(void) {          

	DDRB = 0xFF;								// Set PORTB as output
	PORTB = 0x3C;								// Turn off all 4 LEDs
	DDRC &= (0<<1);								// Set PINC1 as input
	     
		 
	while(1){      
		if ((PINC & (1<<PINC1)) == 0) {			// Check if PINC1 is low (pressed)
		PORTB &= ~(1<<PB2);						// Turn led on
		_delay_ms(1750);						// Wait 1.75s
		PORTB |= (1<<PB2);						// Turn led off
		}
	}
	return 1;
}