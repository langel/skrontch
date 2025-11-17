


	MAC STATE_SET
	lda #<{1}
	sta state_jmp_lo
	lda #>{1}
	sta state_jmp_hi
	ENDM


state_jmp_to: subroutine
	; lets us rts back to vectors
	; banking could be added here
	jmp (state_jmp_lo)
	
