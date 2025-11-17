
ent_ball_spawn: subroutine
	jsr ent_find_slot
	txa
	bmi .done
	lda #ent_ball_id
	sta ent_type,x
	; load baddie position
	lda temp05
	sta ent_x,x
	lda temp06
	sta ent_y,x
.done
	rts


ent_ball_update: subroutine

	lda ent_x_lo,x
	clc
	adc #$97
	sta ent_x_lo,x
	lda ent_x,x
	adc #$01
	sta ent_x,x

	lda wtf
	and #$07
	bne .dont_spawn_laser
	lda ent_x,x
	sta temp05
	lda ent_y,x
	sta temp06
	jsr ent_laser_spawn
	ldx ent_slot
.dont_spawn_laser


ent_ball_render: subroutine

	lda ent_x,x
	sta spr_x,y
	lda ent_y,x
	sta spr_y,y

	lda #$00
	sta spr_a,y

	lda #$30
	sta spr_p,y

	iny
	iny
	iny
	iny

	rts
