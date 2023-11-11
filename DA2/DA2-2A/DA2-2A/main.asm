;
; DA2-2A.asm
;
; Created: 3/3/2023 11:34:58 PM
; Author : Tony Alhwayek

.ORG 0	; Location for reset
JMP MAIN
.ORG 0x06 ; Location for external interrupt 0
JMP INT0_ISR

MAIN:
; Initializing the stack
LDI R20,HIGH(RAMEND)
OUT SPH,R20
LDI R20,LOW(RAMEND)
OUT SPL,R20


; Set PORTB as output
LDI R17, 0xFF			; Load R17 with 0xFF
OUT DDRB, R17			; Set PortB as output
LDI R16, 0b00111100		; Start by turning off all LEDs
OUT PORTB, R16			; Turn off the LEDs	


// INTERRUPT STARTS HERE
LDI R20,0x2				; Make INT0 falling edge triggered
STS EICRA,R20			; Store R20 in EIRCA (falling-edge triggered)
SBI PORTD,2				; Enable PORTD.2 pull-up resistor
LDI R20,1<<INT0			; enable INT0
OUT EIMSK,R20			; Output R20 through EIMSK
SEI						; Enable global interrupts
	
HERE: JMP HERE

INT0_ISR:				; Interrupt label				
	CBI PORTB, 5		; Turn on the LED
	RCALL delay_35s		; Call the 3.5 second delay
	RETI				; Return from Interrupt

	

delay_35s:				; 2 * 1.75s = 3.5s delay
	LDI R24, 2			; Load 2 into R24 (2 loops)
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
	dec r24
	brne delay_175s
	SBI PORTB, 5	; Turn off the LED
	ret


	
