
state_explore_palette:
	hex 0c 11 12 1c
	hex 0c 0c 21 3c
	hex 0c 06 16 27
	hex 0c 0c 21 3c
	hex 0c 0c 21 3c
	hex 0c 0c 21 3c
	hex 0c 0c 21 3c
	hex 0c 0c 21 3c


state_explore_init: subroutine
	STATE_SET state_explore_update

	jsr render_disable

	lda #$3f
	sta PPU_ADDR
	lda #$00
	sta PPU_ADDR
	ldx #$00
.pal_load_loop
	lda state_explore_palette,x
	sta PPU_DATA
	inx
	cpx #$20
	bne .pal_load_loop

	lda #<main_layout_nam
	sta temp00
	lda #>main_layout_nam
	sta temp01
	lda #$20
	jsr nametable_load

	jsr render_enable

	rts


state_explore_update: subroutine
	rts
