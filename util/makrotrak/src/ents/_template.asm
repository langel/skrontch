
ent_template_spawn: subroutine
	jsr state_explore_ent_find_slot
	txa
	bmi .done
	lda #ent_template_id
	sta ent_type,x
	; load baddie position
	lda temp05
	sta ent_x,x
	lda temp06
	sta ent_y,x
.done
	jmp state_explore_ent_spawn_return


ent_template_update: subroutine


ent_template_render: subroutine
	ldy ent_spr_ptr
	rts
	
	

