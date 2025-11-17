
ent_laser_spawn: subroutine
	jsr ent_find_slot
	txa
	bmi .done
	lda #ent_laser_id
	sta ent_type,x
	; load baddie position
	lda temp05
	sta ent_x,x
	lda temp06
	sta ent_y,x
.done
	rts


ent_laser_update: subroutine


	lda ent_y_lo,x
	clc
	adc #$27
	sta ent_y_lo,x
	lda ent_y,x
	adc #$02
	sta ent_y,x

	cmp #249
	bne .dont_despawn
	ent_despawn
	rts
.dont_despawn


ent_laser_render: subroutine

	lda ent_x,x
	sta spr_x,y
	lda ent_y,x
	sta spr_y,y

	lda #$00
	sta spr_a,y

	lda #$21
	sta spr_p,y

	iny
	iny
	iny
	iny

	rts
	
	

