;
; DA2-1A.asm
;
; Created: 2/18/2023 1:41:03 PM
; Author : Tony Alhwayek
;

; Set PORTB as output
LDI R17, 0xFF			; Load R17 with 0xFF
OUT DDRB, R17			; Set PortB as output
LDI R16, 0b00111100		; Start by turning off all LEDs
OUT PORTB, R16			; Turn off the LEDs	


; Poll for button press
AGAIN:
	SBIC PINC, 1		; Check if PINC1 is clear (since the button is active low)
	RJMP AGAIN			; Poll while waiting for button press
	CBI PORTB, 2		; Turn on the LED
	RCALL DELAY_175s	; Call 1.75s delay
	
	HERE: JMP HERE

delay_175s:				; 10 * 0.175s = 1.75s
	LDI R19, 10			; Load 10 into R19 (10 loops)
delay_0175s:			; 175 * 1ms = 0.175s
	LDI R18, 175		; Load 175 into R18 (175 loops)
convert_to_16MHz:		; Loop the 1ms loop 16 times because this is 16MHz and the loop was meant for 1MHz
	LDI R17, 16			; Load 16 into R17 (16 loops)
delay_1ms:
	push r16			; save the value in r16
	ldi r16,99			; accounts for overhead of 12 cycles.
	delay_1ms1:			; 10 us/loop
	nop					; [1 cycle ]
	nop
	nop
	nop
	nop
	nop
	nop
	dec r16
	brne delay_1ms1
	NOP
	NOP
	pop r16
	dec r17
	brne delay_1ms
	NOP
	NOP
	NOP
	NOP
	NOP
	NOP
	NOP
	NOP
	dec r18
	brne convert_to_16MHz
	NOP
	NOP
	NOP
	NOP
	NOP
	NOP
	dec r19
	brne delay_0175s
	SBI PORTB, 2		; Turn off the LED at PORTB.2
	ret
