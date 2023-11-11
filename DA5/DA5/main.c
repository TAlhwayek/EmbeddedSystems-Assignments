/*
 * DA5.c
 *
 * Created: 4/3/2023 10:44:31 AM
 * Author : Tony Alhwayek
 */ 

#define F_CPU 16000000UL

#include <stdio.h>
#include <stdlib.h>
#include <avr/io.h>
#include <util/delay.h>
#include <avr/interrupt.h>
#include <string.h>

#define BAUDRATE 9600
#define BAUD_PRESCALLER (((F_CPU / (BAUDRATE * 16UL))) - 1)
char buffer[5];                //Output of the itoa function

void USART_init(void){
	
	UBRR0H = (uint8_t)(BAUD_PRESCALLER>>8);
	UBRR0L = (uint8_t)(BAUD_PRESCALLER);
	UCSR0B = (1<<TXEN0);
	UCSR0C = (3<<UCSZ00);
}

void USART_send( unsigned char data){
	
	while(!(UCSR0A & (1<<UDRE0)));
	UDR0 = data;
	
}

void USART_putstring(char* StringPtr){
	
	while(*StringPtr != 0x00){
		USART_send(*StringPtr);
	StringPtr++;}
	
}

#define  Trigger_pin	PB1	/* Trigger pin */

int TimerOverflow = 0;		// Initialize timer overflow counter

ISR(TIMER1_OVF_vect)
{
	TimerOverflow++;		/* Increment Timer Overflow count */
}

//Simple Wait Function
void Wait()
{
	uint8_t i;
	for(i=0;i<50;i++)
	{
		_delay_loop_2(0);
		_delay_loop_2(0);
		_delay_loop_2(0);
	}

}


int main(void)
{
	
	char string[10];			// Used for distance as a string
	char string2[10];			// Used for angle as a string
	char result[20];			// Used to send everything into USART
	long count;
	double distance;			// Used for distance
	double angle = 0;			// Used for angle
	
	DDRB = 0x02;			/* Make trigger pin as output */
	/* PB0 is the Echo Pin & PB1 is the Trigger in */
	USART_init();
	
	//Configure TIMER3 for Servo Motor
	TCCR3A|=(1<<COM3A1)|(1<<COM3B1)|(1<<WGM31);        //NON Inverted PWM
	TCCR3B|=(1<<WGM33)|(1<<WGM32)|(1<<CS31)|(1<<CS30); //PRESCALER=64 MODE 14(FAST PWM)
	ICR3=4999;  //fPWM=50Hz (Period = 20ms Standard).
	DDRD|=(1<<PD0);   //PWM Pins as Out
	
	sei();					/* Enable global interrupt */
	TIMSK1 = (1 << TOIE1);	/* Enable Timer1 overflow interrupts */
	TCCR1A = 0;				/* Set all bit to zero Normal operation */

	while(1)
	{

		// FUNCTIONAL SERVO CODE
		OCR3A=97;   //0 degrees
		Wait();
		angle = 0; // Reset angle each time
		// Keep looping until 180 degrees
		for(int i = 0; i < 88; i++) {
			OCR3A += 5;					// Increment OCR3A by 5 each loop
				
			PORTB |= (1 << Trigger_pin);/* Give 10us trigger pulse on trig. pin to HC-SR04 */
			_delay_us(10);
			PORTB &= (~(1 << Trigger_pin));
			
			TCNT1 = 0;			/* Clear Timer counter */
			TCCR1B = 0x41;		/* Setting for capture rising edge, No pre-scaler*/
			TIFR1 = 1<<ICF1;		/* Clear ICP flag (Input Capture flag) */
			TIFR1 = 1<<TOV1;		/* Clear Timer Overflow flag */

			/*Calculate width of Echo by Input Capture (ICP) on PortD PD6 */
			
			while ((TIFR1 & (1 << ICF1)) == 0);	/* Wait for rising edge */
			TCNT1 = 0;			/* Clear Timer counter */
			TCCR1B = 0x01;		/* Setting for capture falling edge, No pre-scaler */
			TIFR1 = 1<<ICF1;		/* Clear ICP flag (Input Capture flag) */
			TIFR1 = 1<<TOV1;		/* Clear Timer Overflow flag */
			TimerOverflow = 0;	/* Clear Timer overflow count */

			while ((TIFR1 & (1 << ICF1)) == 0); /* Wait for falling edge */
			count = ICR1 + (65535 * TimerOverflow);	/* Take value of capture register */
			/* 8MHz Timer freq, sound speed =343 m/s, calculation mentioned in doc. */
			distance = (double)count / (58*16);

			dtostrf(distance, 2, 0, string); // Convert distance to string
			// This checks if angle needs 1, 2 or 3 digits
			// Since having 3 digits only was giving me 0 degrees when < 100 degrees
			if(angle >= 100)					 // If 3 digits
				dtostrf(angle, 3, 0, string2);	 // Convert angle to string
			else if(angle < 100 && angle >= 10)  // If 2 digits
				dtostrf(angle, 2, 0, string2);	 // Convert angle to string
			else if(angle < 10)					 // If 1 digit
				dtostrf(angle, 1, 0, string2);	 // Convert angle to string
			strcpy(result, string2);	// Add angle to result
			strcat(result, ",");		// Add a comma
			strcat(result, string);		// Add distance to result
			strcat(result, ".");		// Add a period
			USART_putstring(result);	// Send result through USART
			_delay_ms(200);				// 200ms delay
			angle += 2.06896551724;		// Increment angle each loop
		}
	}
}