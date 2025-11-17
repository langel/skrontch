
ents_max  eqm #$1f


ent_find_slot: subroutine
	; returns empty slot in x
	; x = 0xff if none found
	ldx #$00
.loop
	lda ent_type,x
	beq .slot_found
	inx
	cpx #ents_max
	bne .loop
	ldx #$ff
.slot_found
	rts



ents_update: subroutine
	; debug visualization on
	;lda #%00011111 ; b/w
	;lda #%11111110 ; emph
	;sta PPU_MASK

	; don't corrupt y in ent render functions
	ldy #$10
	sty ent_spr_ptr

	lda ent_slot_start
	clc
	adc #$07
	and #$1f
	sta ent_slot_start
	sta ent_slot

	; sprites_clear
	lda #$ff
	ldx #$00
.loop
	sta $0200,x
	inx
	inx
	inx
	inx
	bne .loop

	lda wtf
	lsr
	and #$01
	beq .update_forward_loop
	bne .update_backward_loop

.update_forward_loop
	ldx ent_slot
	lda ent_type,x
	beq .skip_forward_ent_slot
	jsr ents_update_jump
	sty ent_spr_ptr
.skip_forward_ent_slot
	inc ent_slot
	lda ent_slot
	and #ents_max
	sta ent_slot
	cmp ent_slot_start
	bne .update_forward_loop
	jmp .updates_done

.update_backward_loop
	ldx ent_slot
	lda ent_type,x
	beq .skip_backward_ent_slot
	jsr ents_update_jump
	sty ent_spr_ptr
.skip_backward_ent_slot
	dec ent_slot
	lda ent_slot
	bpl .dont_reset_ent_slot
	lda #ents_max
	sta ent_slot
.dont_reset_ent_slot
	cmp ent_slot_start
	bne .update_backward_loop

.updates_done
	; debug visualization off
	;lda #%00011110
	;sta PPU_MASK
	rts


ents_update_jump: subroutine
	tax
	lda ent_update_lo,x
	sta temp00
	lda ent_update_hi,x
	sta temp01
	ldx ent_slot
	ldy ent_spr_ptr
	jmp (temp00)
	


	MAC ent_despawn
	lda #$00
	sta ent_type,x
	sta ent_spawn,x
	sta ent_x_hi,x
	sta ent_x,x
	sta ent_x_lo,x
	sta ent_y,x
	sta ent_y_lo,x
	sta ent_h,x
	sta ent_h_lo,x
	sta ent_v,x
	sta ent_v_lo,x
	sta ent_dir,x
	sta ent_hp,x
	sta ent_dmg,x
	sta ent_hit,x
	sta ent_r0,x
	sta ent_r1,x
	sta ent_r2,x
	sta ent_r3,x
	sta ent_r4,x
	sta ent_r5,x
	sta ent_r6,x
	sta ent_r7,x
	sta ent_r8,x
	ENDM


