
	processor 6502

	include "definitions.asm"
	include "zero_page.asm"

	; HEADER
	; mapper, PRGs (16k), CHRs (8k), mirror, ram expansion
	NES_HEADER 0, 2, 1, NES_MIRR_HORIZ, 0

	seg DATA_BANKS
	org $08000 
	rorg $8000
main_layout_nam:
	include "vectors.asm"
	include "common.asm"

	include "ent.asm"
	include "ent_tables.asm"
	include "ents/ball.asm"
	include "ents/laser.asm"

	include "state.asm"
	include "states/explore.asm"
	include "states/map.asm"
	include "states/title.asm"
	include "states/win.asm"

	org $0e000 
	rorg $e000


	;;;;; CPU VECTORS
	seg VECTORS
	org $0fffa	
	rorg $fffa ; start at address $fffa
	.word nmi_handler	; $fffa vblank nmi
	.word cart_start	; $fffc reset
	.word nmi_handler	; $fffe irq / brk


	;;;;; GRAPHX
grfx_offset EQM $10000

	org $0000+grfx_offset
	incbin "./assets/ascii.chr"
	incbin "./assets/ascii.chr"

;	org $3fff+grfx_offset
;	byte 0

