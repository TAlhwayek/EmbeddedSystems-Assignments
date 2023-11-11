/*
 * DA4.c
 *
 * Created: 3/26/2023 2:46:05 AM
 * Author : tonya
 */ 

#include <avr/io.h>
#include <avr/interrupt.h>
#include <stdio.h>
#include <stdbool.h>

#ifndef F_CPU
#define F_CPU 16000000UL
#endif

#ifndef BAUD
#define BAUD 57600
#endif
#include <util/setbaud.h>

void uart_putchar(char c, FILE *stream);
char uart_getchar(FILE *stream);

void uart_init(void);

/* http://www.ermicro.com/blog/?p=325 */

FILE uart_output = FDEV_SETUP_STREAM(uart_putchar, NULL, _FDEV_SETUP_WRITE);
FILE uart_input = FDEV_SETUP_STREAM(NULL, uart_getchar, _FDEV_SETUP_READ);

/* http://www.cs.mun.ca/~rod/Winter2007/4723/notes/serial/serial.html */

void uart_init(void) {
	UBRR0H = UBRRH_VALUE;
	UBRR0L = UBRRL_VALUE;
	
	#if USE_2X
	UCSR0A |= _BV(U2X0);
	#else
	UCSR0A &= ~(_BV(U2X0));
	#endif

	UCSR0C = _BV(UCSZ01) | _BV(UCSZ00); /* 8-bit data */
	UCSR0B = _BV(RXEN0) | _BV(TXEN0);   /* Enable RX and TX */
	UCSR0B |= ( 1<< RXCIE0); 
}

void uart_putchar(char c, FILE *stream) {
	if (c == '\n') {
		uart_putchar('\r', stream);
	}
	loop_until_bit_is_set(UCSR0A, UDRE0);
	UDR0 = c;
}

// Variables needed
// Input is used to stored the input
// ADCon is used to check if the ADC should be printing
// Receive is used to check if an character was received
// Blink is used to check if the LED should be blinking
volatile char input;
volatile bool ADCon;
volatile bool receive;
volatile bool blink = false;

char uart_getchar(FILE *stream) {
	loop_until_bit_is_set(UCSR0A, RXC0);
	return UDR0;
}

// Function that prints the help menu
void printMenu(void) {
	printf("\nHelp Menu: \n");
	printf("o - Turn on PB5\n");
	printf("O - Turn off PB5\n");
	printf("b - Make PB3 blink for 0.32s\n");
	printf("P - Turn off PB3\n");
	printf("a - Get potentiometer value in volts\n");
	printf("A - Stop getting potentiometer values\n");
	printf("h - Print this help menu again\n");
}

void InitTimer1(void)
{
	//Set Initial Timer value
	TCNT1 = 40525;
}

void StartTimer1(void)
{
	// set prescaler to 64 and start timer1
	TCCR1B = (1 << CS11) | (1 << CS10); 
}

// Stop timer1
void StopTimer(void)
{
	TCCR1B &= ~(1 << CS11);
	TIMSK1 &= ~(1 << OCIE1A);
}

// Set ADC by passing a value as a parameter
void SetADCChannel(uint8_t ADCchannel)
{
	//select ADC channel with safety mask
	ADMUX = (ADMUX & 0xF0) | (ADCchannel & 0x0F);
}

// Start ADC
void StartADC(void)
{
	ADCSRA |= (1 << ADSC);
}

// Disable ADC
void DisableADC(void)
{
	ADCSRA &= ~((1 << ADEN) | (1 << ADIE));
}

void InitADC(void)
{
	// Select Vref=Avcc and set left adjust result
	ADMUX |= (1 << REFS0);
	// set prescaler to 32, enable autotriggering, enable ADC interrupt
	// and enable ADC
	ADCSRA |= (1 << ADPS2) | (1 << ADPS0) | (1 << ADATE) | (1 << ADEN);
	// set ADC trigger source - Timer1 compare match A
	ADCSRB |= (1 << ADTS2) | (1 << ADTS1);
}

// USART Interrupt
// This gets the characters from the terminal
ISR (USART0_RX_vect) {
	input =  getchar();
	receive = true;
}

int main(void) {
	
	DDRB = 0x28;										// Set PORTB3,5 as outputs
	PORTB = 0x28;										// Turn PORTB3,5 off (they start on)
	float ADCValue = 0.0;								// Used to print voltage as float

	uart_init();										// Initialize UART
	stdout = &uart_output;								// UART output stream
	stdin  = &uart_input;								// UART input stream
	printMenu();										// Print help screen on boot
	sei();												// Enable global interrupts
	
	while(1) {
		
		// On each loop, check if blink is still set
		// If it's still set, let the LED blink
		if (blink)										// Keep looping while blink is true
		{
			// 0.32s delay
			for(int i = 0; i < 1000; i++) {				// Loop 1000 times
				int counter = 0;						// Reset counter each loop
				while (counter < 20) {					// Repeat 20 times
					while ((TIFR0 & 0x01) == 0);		// Wait for timer0 overflow
					counter++;							// Increment counter after every timer0 overflow
					TIFR0=0x01;							// Reset the timer0 overflow flag
				}
			}
			PORTB ^= (1<<DDB3);							// Toggle LED at PB3
		}
		
		// Update ADC value and print
		if (ADCon) {									// Check if ADCon is true
			if ((TIFR1 & 0x01) == 1) {					// Check for timer1 overflow (auto-trigger)
			ADCValue = (float)(ADC) * 5.00 / 1023.00;	// Convert potentiometer value to voltage
			printf("ADC value = %.2f V\n", ADCValue);	// Print voltage
			TCNT1 = 40525;								// Reset timer1
			TIFR1= 0x01;								// Reset timer1 overflow flag
			}
		}
		
		// This is where the switch statement starts
		if (receive) {									// When a character is received
		printf("You wrote %c\n", input);				// Show what the user sent
		switch (input) {								// Switch statement based on input
			
			case 'h':				// When h is received
			printMenu();			// Print help screen
			break;
	
			case 'o':				// When o is received
			PORTB &= ~(1 << DDB5);	// Turn on PB5
			break;  
			
			case 'O':				// When O is received				
			PORTB |= (1 << DDB5);	// Turn off PB5
			break;
			
			case 'b':									// When b is received
			TCCR0A = 0;				// Normal Operation
			TCNT0 = 0x00;			// Start the timer
			TCCR0B |= (1 << CS00);	// Set prescaler to 1 and start the timer
			blink = true;			// Allow the LED to blink
			break;
			
			case 'P':				// When P is received (pressed)
			PORTB |= (1 << DDB3);	// Turn off the LED
			blink = false;			// Stop LED from blinking
			break;
			
			case 'a':				// When a is pressed
			InitADC();				// Initialize ADC
			SetADCChannel(0);		// Select ADC0
			InitTimer1();			// Prepare timer1
			StartTimer1();			// Start timer 1
			StartADC();				// Start ADC conversion
			ADCon = true;			// Enable ADC printing
			break;
			
			case 'A':				// When A is pressed
			ADCon = false;			// Stop ADC printing
			StopTimer();			// Stop Timer1
			DisableADC();			// Disable ADC
			break;

			// If invalid command is received
			default:
			printf("Command not recognized\r\n");
			break;
			}
			receive = false;
		}
	}

	return 0;
}
