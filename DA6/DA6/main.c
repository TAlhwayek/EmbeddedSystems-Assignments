/*
 * DA6.c
 *
 * Created: 4/17/2023 9:48:20 PM
 * Author : Tony Alhwayek
 */ 

#define F_CPU 16000000UL /* Define CPU Frequency e.g. here its 8MHz */
#include <avr/interrupt.h>
#include <avr/io.h>
#include <avr/pgmspace.h>
#include <stdio.h>
#include <util/delay.h>

#define BAUD_PRESCALE (((F_CPU / (BAUDRATE * 16UL))) - 1)	/* Define prescale value */
#define SHIFT_REGISTER DDRB
#define SHIFT_PORT PORTB
#define DATA (1<<PB3)		//MOSI (SI)
#define LATCH (1<<PB2)		//SS   (RCK)
#define CLOCK (1<<PB5)		//SCK  (SCK)

void USART_Init(unsigned long BAUDRATE)				/* USART initialize function */
{
	UCSR0B |= (1 << RXEN0) | (1 << TXEN0);			/* Enable USART transmitter and receiver */
	UCSR0C |= (1 << UCSZ00) | (1 << UCSZ01);		/* Write USCRC for 8 bit data and 1 stop bit */
	UBRR0L = BAUD_PRESCALE;							/* Load UBRRL with lower 8 bit of prescale value */
	UBRR0H = (BAUD_PRESCALE >> 8);					/* Load UBRRH with upper 8 bit of prescale value */
}

char USART_RxChar()									/* Data receiving function */
{
	while (!(UCSR0A & (1 << RXC0)));				/* Wait until new data receive */
	return(UDR0);									/* Get and return received data */
}

void USART_TxChar(char data)						/* Data transmitting function */
{
	UDR0 = data;									/* Write data to be transmitting in UDR */
	while (!(UCSR0A & (1<<UDRE0)));					/* Wait until data transmit and buffer get empty */
}

void USART_SendString(char *str)					/* Send string of USART data function */
{
	int i=0;
	while (str[i]!=0)
	{
		USART_TxChar(str[i]);						/* Send each char of string till the NULL */
		i++;
	}
}

void ADC_Init() /* ADC Initialization function */
{
	DDRC = 0x00;   /* Make ADC port as input */
	ADCSRA = 0x87; /* Enable ADC, with freq/128  */
	ADMUX = 0x40;  /* Vref: Avcc, ADC channel: 0 */
}

int ADC_Read(char channel) /* ADC Read function */
{
	ADMUX = 0x40 | (channel & 0x07); /* set input channel to read */
	ADCSRA |= (1 << ADSC);           /* Start ADC conversion */
	while (!(ADCSRA & (1 << ADIF)))
	; /* Wait until end of conversion by polling ADC interrupt flag */
	ADCSRA |= (1 << ADIF); /* Clear interrupt flag */
	_delay_us(1);          /* Wait a little bit */
	return ADCW;           /* Return ADC word */
}

volatile uint32_t revTickAvg;	// Average ticks
volatile uint32_t revTick;		// Ticks per revolution
volatile uint32_t revCtr;		// Total elapsed revolutions
volatile uint16_t T3Ovs2;		// Overflows for small rotations

// Initialize timer
void InitTimer3(void) {

	// Set Initial Timer value
	TCNT3 = 0;
	////First capture on rising edge
	TCCR3A = 0;
	TCCR3B = (0 << ICNC3) | (1 << ICES3);
	TCCR3C = 0;
	// Interrupt setup
	// ICIE1: Input capture
	// TOIE1: Timer1 overflow
	TIFR3 = (1 << ICF3) | (1 << TOV3);    // clear pending
	TIMSK3 = (1 << ICIE3) | (1 << TOIE3); // and enable
}

void StartTimer3(void) {
	// Start timer with a pre-scaler = 64
	TCCR3B |= (1 << CS30);
	// Enable global interrupts
	sei();
}

volatile uint32_t tickv, ticks;
// capture ISR
ISR(TIMER3_CAPT_vect) {
	tickv = ICR3; // save duration of last revolution
	revTickAvg = (uint32_t)(tickv) + ((uint32_t)T3Ovs2 * 0x10000L);
	revCtr++;  // add to revolution count
	TCNT3 = 0; // restart timer for next revolution
	T3Ovs2 = 0;
}
// Overflow ISR
ISR(TIMER3_OVF_vect) {
	// increment overflow counter
	T3Ovs2++;
}

void init_IO(void){
	//Setup IO
	SHIFT_REGISTER |= (DATA | LATCH | CLOCK);	//Set control pins as outputs
	SHIFT_PORT &= ~(DATA | LATCH | CLOCK);		//Set control pins low
}

void init_SPI(void){
	//Setup SPI
	SPCR0 = (1<<SPE) | (1<<MSTR);	//Start SPI as Master
}

void spi_send(unsigned char byte){
	SPDR0 = byte;			//Shift in some data
	while(!(SPSR0 & (1<<SPIF)));	//Wait for SPI process to finish
}

/* Segment byte maps for numbers 0 to 9 */
const uint8_t SEGMENT_MAP[] = {0xC0, 0xF9, 0xA4, 0xB0, 0x99,
0x92, 0x82, 0xF8, 0X80, 0X90};


int main(void) {
	
	// For USART printing
	// (used to show atmel output in DA PDF)
	char outs[72];
	USART_Init(57600);
	USART_SendString("Connected!\r\n");
	USART_SendString("TIMER3 ICP Running \r\n");
	
	// Initialize and start timer3
	InitTimer3();
	StartTimer3();

	// Initialize SPI
	init_IO();
	init_SPI();
	
	DDRD |= (1 << DDD6); /* Make OC0 pin as Output */
	ADC_Init();								/* Initialize ADC */
	TCNT0 = 0;								/* Set timer0 count zero */
	TCCR0A |= (1<<WGM00)|(1<<WGM01)|(1<<COM0A1);
	TCCR0B |= (1<<CS00)|(1<<CS02);/* Set Fast PWM with Fosc/64 Timer0 clock */
	sei();                 /* Enable Global Interrupt */
	
	OCR0A = 80;
	
	while (1) {

		// Update time from ADC
		OCR0A = ADC_Read(0)/4;
		
		// Get RPM
		int RPM = (60 * 8000000) / (64 * revTickAvg);
		
		// Print RPM 
		USART_SendString("Ticks: ");
		snprintf(outs, sizeof(outs), "%f ", (float)revTickAvg);
		USART_SendString(outs);
		USART_SendString("   RPM: ");
		snprintf(outs, sizeof(outs), "%d ", (short)RPM);
		USART_SendString(outs);
		USART_SendString(" \r\n");
		
		// Initialize 7-segment displays
		SHIFT_PORT &= ~LATCH;
		
		// Sending RPM value to 7-segment displays
		// Send tens digit to 1st 7-segment display
		spi_send((unsigned char)SEGMENT_MAP[RPM / 10]);
		spi_send((unsigned char)0xF1);
		//Toggle latch to copy data to the storage register
		SHIFT_PORT |= LATCH;
		SHIFT_PORT &= ~LATCH;
		// Slight delay to see the digit
		_delay_ms(100);
		
		// Send ones digit to 2nd 7-segment display
		spi_send((unsigned char)SEGMENT_MAP[RPM % 10]);
		spi_send((unsigned char)0xF2);
		//Toggle latch to copy data to the storage register
		SHIFT_PORT |= LATCH;
		SHIFT_PORT &= ~LATCH;
		// Slight delay to see the digit
		_delay_ms(100);		
	}
	
	return 0;
}