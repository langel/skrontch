
state_title_palette:
	hex 0f 01 22 35
	hex 0f 01 22 35
	hex 0f 01 22 35
	hex 0f 01 22 35
	hex 0f 01 22 35
	hex 0f 01 22 35
	hex 0f 01 22 35
	hex 0f 01 22 35


str_title:
	byte "MAKROTRAK"


state_title_init: subroutine
	STATE_SET state_title_update

	jsr render_disable

	lda #$3f
	sta PPU_ADDR
	lda #$00
	sta PPU_ADDR
	ldx #$00
.pal_load_loop
	lda state_title_palette,x
	sta PPU_DATA
	inx
	cpx #$20
	bne .pal_load_loop

	lda #$20
	sta temp00
	lda #$00
	sta temp01
	lda #$00
	jsr nametable_fill

	lda #$21
	sta PPU_ADDR
	lda #$8c
	sta PPU_ADDR
	ldx #$00
.title_loop
	lda str_title,x
	sta PPU_DATA
	inx
	cpx #$09
	bne .title_loop

	jsr render_enable

	lda #$00
	sta temp05
	lda #$20
	sta temp06
	jsr ent_ball_spawn

	rts



state_title_update: subroutine
	lda controls_d
	beq .do_nothing
	;jsr state_explore_init
.do_nothing

	jsr ents_update

	lda controls
	beq .no_triangle
.do_triangle
	lda #$7f
	sta $4008
	lda #$f8
	sta $400a
	lda #$03
	ora #%11111000
	sta $400b
	lda #$80
	sta spr_x+$fc
	sta spr_y+$fc
	lda #$2a
	sta spr_p+$fc
	jmp .triangle_done
.no_triangle
	lda #$00
	sta $4008
	sta $400b
	lda #$e0
	sta spr_y+$fc
.triangle_done


	rts
