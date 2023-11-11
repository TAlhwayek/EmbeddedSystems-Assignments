;
; DA1.asm
;
; Created: 2/9/2023 8:50:02 PM
; Author : Tony Alhwayek
;

; Initializing the stack pointer
LDI R20, HIGH(RAMEND)
OUT SPH, R20
LDI R20, LOW(RAMEND)
OUT SPL, R20


.SET SRAM   = 1024		; Use the SRAM's middle address
.SET EEPROM = 512		; Use the EEPROM 's middle address

LDI R23, 0				; Used to store data in EEPROM
LDI R16, 10				; Used to loop 10 times
LDI R17, 0				; Used for the carry from adding the upper bits
LDI R18, 0				; Used for adding the upper bits
LDI R19, 0				; Used for adding the lower bits


LDI ZL, LOW(MYDATA<<1)  ; Shift left the lower bits of MYDATA's address, and store in ZL
LDI ZH, HIGH(MYDATA<<1) ; Shift left the upper bits of MYDATA's address, and store in ZH

L1:
	LPM R22, Z+			; get the low 8 bits from MYDATA
	LPM R21, Z+			; get the high 8 bits from MYDATA


	ADD R19, R22		; Sum of lower bits in R19
	ADC R18, R21		; Sum of higher bits in R18
	ADC R17, R0         ; Store the carry from the upper bits into this (Carry + 0)
	DEC R16				; Decrement R16 so this only loops 10 times
	BRNE L1				; When R16 = 0, end the loop



; Store in SRAM
LDI XL, LOW(SRAM)		; Store lower bits of SRAM address into YL
LDI XH, HIGH(SRAM)		; Store upper bits of SRAM address into YH
ST X+, R17				; Store the carry of the upper bits of the sum
ST X+, R18				; Store the upper bits of the sum
ST X+, R19				; Store the lower bits of the sum


; Store in EEPROM
LDI YH, HIGH(0x200)		; Store the upper bits of the address 0x200 in YH
LDI YL, LOW(0x200)		; Store the lower bits of the address 0x200 in YL

MOV R23, R17			; Store value in the temp register to be saved in EEPROM
CALL STORE_IN_EEPROM	; Store in EEPROM
INC YL					; Increment Y pointer

MOV R23, R18			; Store value in the temp register to be saved in EEPROM
CALL STORE_IN_EEPROM	; Store in EEPROM
INC YL					; Increment Y pointer

MOV R23, R19			; Store value in the temp register to be saved in EEPROM
CALL STORE_IN_EEPROM	; Store in EEPROM
INC YL					; Increment Y pointer

HERE: RJMP HERE			; End loop

STORE_IN_EEPROM:
	SBIC EECR, EEPE			; Check if EECR and EEPE are clear
	RJMP STORE_IN_EEPROM    ; Jump back while waiting for cleared bits
	OUT EEARH,YH			; Write the upper bits of Y into the EEPROM’s high address register
	OUT EEARL,YL			; Write the lower bits of Y into the EEPROM’s low address register
	OUT EEDR,R23			; Write R23 to the EEPROM’s data register
	SBI EECR,EEMPE			; Set EECR, EEMPE
	SBI EECR,EEPE			; SET EECR, EEPE
	RET						; Return to where the function was called



.ORG 0xF77					; Start at memory location 0x1EEE
; Storing 10 16-bit numbers
MYDATA: .DW 0x6634, 0x5535, 0x4236, 0x3237, 0xA238, 0x1239, 0x123A, 0x123B, 0x123C, 0x123D




