#include <avr/io.h>

;
; RGB LED GAME CONTR.asm
;
; Created: 26/03/2020 22:58:49
; Author : simon
;


;
; rgb led.asm
;
; Created: 26/03/2020 22:55:02
; Author : simon, eric
;


.section .data
	ledarray: .skip 12   ; declare an array of 3 bytes (RGB) for the 4 leds



.section .text
.global DriverPL9823BitbangSet
DriverPL9823BitbangSet:	
		push r0
		push r29
		
		ldi r29,8		
		;init ledarray 
		ldi r30,lo8(ledarray) ; Load Z register low 
		ldi r31,hi8(ledarray) ; Load Z register high 
		; fill ledarray
		st Z+,r22 ;R1  
		st Z+,r23 ;G1
		st Z+,r24 ;B1  
 
		st Z+,r18 ;R2  
		st Z+,r19 ;G2
		st Z+,r20 ;B2
		  
		st Z+,r14 ;R3 
		st Z+,r15 ;G3  
		st Z+,r16 ;B3
  
		st Z+,r10 ;R4 
		st Z+,r11 ;G4  
		st Z+,r12 ;B4
	
		//set Z(pointer) back to start of array
		ldi ZL,lo8(ledarray) ; Load Z register low 
		ldi ZH,hi8(ledarray) ; Load Z register high  
		;**************************************************************************

		;**************************************************************************

startx:		
		//    set Z as startadres of array
		ldi r30,lo8(ledarray) ; Load Z register low 
		ldi r31,hi8(ledarray) ; Load Z register high 

	
						
		;**************************************************************************		
		//reset pl9823  = low pulse of 50 µS

		sts PORTB_OUTCLR,r29			; make output low
		// delay of 200 clock cycles
				ldi r21, 255
				ldi r22, 255
		delay200:	
				dec r21
				brne delay200
				dec r22
				brne delay200
		;**************************************************************************

		sts PORTB_OUTSET,r29			; set PD3 to high
							
									
		ldi r16, 4						; set r16 to number of leds
loop1:
		ldi r17, 3						; set r17 to number of colors
loop2:	sts PORTB_OUTSET,r29			; set PD3 to high
		ld r18, z+						; fetch next byte of array
		ldi r19, 8						; set R19 as counter (8 bits for each color)
	
loop3:
		sts PORTB_OUTSET,r29			; set PD3 to high
		// delay of 7 clock cycles
				nop
				nop
				nop
				nop
				nop
				nop
				nop 
		sbrs r18,7						; if bit 7 is high leave output high
		sts PORTB_OUTCLR,r29			; if not output is low
		nop								; nop's for timing reasons
		nop
		lsl r18							; shift left data
		// delay of 32 clock cycles
				ldi r21, 9
		delay32:	
				dec r21
				brne delay32
			
		sts PORTB_OUTCLR,r29			; make output low
		; delay of 7 clock cycles
				nop
				nop
				nop
				nop
				nop
				nop
				;nop
		dec r19							;bit-counter -1
		brne loop3						; next bit if bit-counter > 0

	
	
		dec r17							; colornr -1
		brne loop2						; next color if colornr > 0
	
		dec r16							; lednr -1
		brne loop1						;next led if lednr> 0
		sts PORTB_OUTSET,r29			; set PD3 to high    idle state of DIN pin of PL9823
		
		pop r29
		pop r0
		clr r1		
end:	ret