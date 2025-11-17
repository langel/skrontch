
cart_start: subroutine
	NES_INIT	; set up stack pointer, turn off PPU
	jsr vsync_wait
	jsr vsync_wait

	; clear ram
	lda #0	
	tax		
.ram_clear_loop
	sta $0,x	
	cpx #$fe	
	bcs .skip_stack
	sta $100,x	
.skip_stack
	lda #$ef
	sta $200,x	
	lda #$00
	sta $300,x	
	sta $400,x	
	sta $500,x	
	sta $600,x	
	sta $700,x	
	inx		
	bne .ram_clear_loop

	jsr render_disable
	lda #$01
	sta rng0
	jsr rng_seed

	cli

	jsr state_title_init
	;jsr state_explore_init


.endless
	jmp .endless	; endless loop







nmi_handler: subroutine

; ~2250 cycles for PPU access (PAL is 7450 cycles)
; "On NTSC, count on being able to copy 160 bytes 
; to nametables or the palette using a moderately 
; unrolled loop"
; write 64 tiles?
; write all palettes
; oam dma

	SAVE_REGS ; 13 cycles

	; enable NMI lockout
	lda nmi_lockout
	cmp #$00
	beq .no_lock
	jmp .nmi_end
.no_lock
	inc nmi_lockout

	; PPU vBLANK STUFF

	; OAM DMA	513 cycles
	lda oam_disable
	bne .oam_skip
	lda #$02
	sta PPU_OAM_DMA
.oam_skip

/*
	; PALETTE RENDER 
	; old method: 12 + 32 x 7 = 236 cycles
	; current method: 90 (+ 64 for sprites) = 154 cycles
	PPU_ADDR_SET $3f00 	; 12 cycles
	tsx			; 14
	stx temp02		; 17
	ldx #$ff		; 19
	txs			; 21
	PPU_POPSLIDE 32  ; 8 cycles each

	ldx temp02		; 88
	txs			; 90
	*/

	; SCROLL POS	17 cycles
	bit PPU_STATUS
	lda scroll_x
	sta PPU_SCROLL
	lda scroll_y
	sta PPU_SCROLL

	; NAMETABLE++	
	lda scroll_nm
	and #$03
	ora #CTRL_NMI|CTRL_SPR_1000
	sta PPU_CTRL
	lda ppu_mask_emph
	;lda #$80
	ora #MASK_BG|MASK_SPR|MASK_SPR_CLIP|MASK_BG_CLIP
	sta PPU_MASK	

	; hope everything above was under
	; ~2250 cycles!

	jsr controller_read

	inc wtf
	
	jsr state_jmp_to

	; disable NMI lockout
	lda #$00
	sta nmi_lockout
	
	
	lda ppu_mask_emph
	ora #MASK_BG|MASK_SPR|MASK_SPR_CLIP|MASK_BG_CLIP
	sta PPU_MASK	

.nmi_end
	RESTORE_REGS ; 16 cycles
	rti

