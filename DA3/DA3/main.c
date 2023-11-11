/*
 * DA3.c
 *
 * Created: 3/11/2023 10:18:41 PM
 * Author : Tony Alhwayek
 */ 


#define F_CPU 16000000UL
#include <avr/io.h>
#include <avr/interrupt.h>

// Counters used
int secondCounter = 0;							// secondCounter is for Task 2
int thirdCounter  = 0;							// thirdCounter is for Task 3

// Task 2 Interrupt
// 1 second
ISR(TIMER1_OVF_vect)
{
	TCNT1 = 0xC1CA;								// Reinitialize the timer
	secondCounter++;							// Increment overflow counter
	if (secondCounter == 1003) {				// 1003 overflows = 0.999s delay
		PORTB ^= (1 << DDB3);					// Toggles the led at PB3
		secondCounter = 0;						// Reset secondCounter
	}
}

// Task 3 Interrupt
// 1.333 seconds
ISR (TIMER2_COMPA_vect)
{
	OCR2A = 165;								// Reinitialize timer
	thirdCounter++;								// Increment overflow counter
	if(thirdCounter == 2008) {					// 2008 overflows = 1.333s delay
		PORTB ^= (1 << DDB2);					// Toggle LED at PB2
		thirdCounter = 0;						// Reset thirdCounter
	}
}

int main(void)
{
	
	DDRB = 0x3C;								// PORTB2 to PORTB5 as output
	PORTB = 0x20;								// Turn off PORTB5
	
	// Task 1 with Timer0
	TCCR0A = 0;									// Normal Operation
	TCNT0 = 0x00;								// Start the timer
	TCCR0B |= (1 << CS00);						// set prescaler to 1 and start the timer
	
	// Task 2 with Timer1
	TCCR1A = 0;									// Initialize the timer
	TCCR1B |= (1 << CS10);						// Set up timer with prescaler = 1
	TCNT1 = 0xC1CA;								// Initialize counter to get a 0.999ms delay
	TIMSK1 |= (1 << TOIE1);						// Enable overflow interrupt
	
	// Task 3 with Timer2
	OCR2A = 165;								// Load Compare Reg value to get 0.666ms
	TCCR2A |= (1 << WGM21);						// Set to CTC Mode
	TIMSK2 |= (1 << OCIE2A);					// Set interrupt on compare match
	TCCR2B |= (1 << CS22);						// set prescaler to 64 and starts PWM
		
	sei();										// Enable global interrupts

	// TASK 1
	while (1)									// Keep looping indefinitely
	{
		// Task 1							
		// 1s delay
		for(int i = 0; i < 2977; i++) {			// Loop 2977 times = 1s delay
			int counter = 0;					// Reset counter each loop
		// 0.333ms delays using a timer
			while (counter < 21) {				// Repeat 21 times
				while ((TIFR0 & 0x01) == 0);	// Wait for timer0 overflow
				counter++;						// Increment counter after every timer0 overflow
				TIFR0=0x01;						// Reset the timer0 overflow flag
			} 
		}
		PORTB ^= (1<<DDB4);						// Toggle LED at PB4
	}
}