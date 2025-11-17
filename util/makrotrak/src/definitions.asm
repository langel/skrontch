

;;;;; CONSTANTS

PPU_CTRL    EQM $2000
PPU_MASK    EQM $2001
PPU_STATUS  EQM $2002
OAM_ADDR    EQM $2003
OAM_DATA    EQM $2004
PPU_SCROLL  EQM $2005
PPU_ADDR    EQM $2006
PPU_DATA    EQM $2007

PPU_OAM_DMA     EQM $4014
DMC_FREQ        EQM $4010
APU_STATUS      EQM $4015
APU_NOISE_VOL   EQM $400C
APU_NOISE_FREQ  EQM $400E
APU_NOISE_TIMER EQM $400F
APU_DMC_CTRL    EQM $4010
APU_CHAN_CTRL   EQM $4015
APU_FRAME       EQM $4017

JOYPAD1         EQM $4016
JOYPAD2         EQM $4017

JOYPAD1         EQM $4016
JOYPAD2         EQM $4017
BUTTON_A        EQM 1 << 7
BUTTON_B        EQM 1 << 6
BUTTON_SELECT   EQM 1 << 5
BUTTON_START    EQM 1 << 4
BUTTON_UP     	 EQM 1 << 3
BUTTON_DOWN   	 EQM 1 << 2
BUTTON_LEFT   	 EQM 1 << 1
BUTTON_RIGHT  	 EQM 1 << 0

; NOTE: I've put this outside of the PPU & APU, because it is a feature
; of the APU that is primarily of use to the PPU.
OAM_DMA         EQM $4014
; OAM local RAM copy goes from $0200-$02FF:
OAM_RAM         EQM $0200

; PPU_CTRL flags
CTRL_NMI        EQM %10000000	; Execute Non-Maskable Interrupt on VBlank
CTRL_8x8        EQM %00000000 	; Use 8x8 Sprites
CTRL_8x16       EQM %00100000 	; Use 8x16 Sprites
CTRL_BG_0000    EQM %00000000 	; Background Pattern Table at $0000 in VRAM
CTRL_BG_1000    EQM %00010000 	; Background Pattern Table at $1000 in VRAM
CTRL_SPR_0000   EQM %00000000 	; Sprite Pattern Table at $0000 in VRAM
CTRL_SPR_1000   EQM %00001000 	; Sprite Pattern Table at $1000 in VRAM
CTRL_INC_1      EQM %00000000 	; Increment PPU Address by 1 (Horizontal rendering)
CTRL_INC_32     EQM %00000100 	; Increment PPU Address by 32 (Vertical rendering)
CTRL_NT_2000    EQM %00000000 	; Name Table Address at $2000
CTRL_NT_2400    EQM %00000001 	; Name Table Address at $2400
CTRL_NT_2800    EQM %00000010 	; Name Table Address at $2800
CTRL_NT_2C00    EQM %00000011 	; Name Table Address at $2C00

; PPU_MASK flags
MASK_TINT_RED   EQM %00100000	; Red Background
MASK_TINT_BLUE  EQM %01000000	; Blue Background
MASK_TINT_GREEN EQM %10000000	; Green Background
MASK_SPR        EQM %00010000 	; Sprites Visible
MASK_BG         EQM %00001000 	; Backgrounds Visible
MASK_SPR_CLIP   EQM %00000100 	; Sprites clipped on left column
MASK_BG_CLIP    EQM %00000010 	; Background clipped on left column
MASK_COLOR      EQM %00000000 	; Display in Color
MASK_MONO       EQM %00000001 	; Display in Monochrome

; read flags
F_BLANK         EQM %10000000 	; VBlank Active
F_SPRITE0       EQM %01000000 	; VBlank hit Sprite 0
F_SCAN8         EQM %00100000 	; More than 8 sprites on current scanline
F_WIGNORE       EQM %00010000 	; VRAM Writes currently ignored.


;;;;; CARTRIDGE FILE HEADER

NES_MIRR_HORIZ	EQM 0
NES_MIRR_VERT	EQM 1
NES_MIRR_QUAD	EQM 8


	MAC NES_HEADER
	seg Header
	org $7ff0
.NES_MAPPER	SET {1}	;mapper number
.NES_PRG_BANKS	SET {2}	;number of 16K PRG banks, change to 2 for NROM256
.NES_CHR_BANKS	SET {3}	;number of 8K CHR banks (0 = RAM)
.NES_MIRRORING	SET {4}	;0 horizontal, 1 vertical, 8 four screen
.NES_RAM_EXP	SET {5}	;0 false, 1 8K extra ram
	byte $4e,$45,$53,$1a ; header
	byte .NES_PRG_BANKS
	byte .NES_CHR_BANKS
	byte .NES_MIRRORING|(.NES_MAPPER<<4)|.NES_RAM_EXP
	byte .NES_MAPPER&$f0
	byte 0,0,0,0,0,0,0,0 ; reserved, set to zero
	seg Code
	org $8000
	ENDM


	MAC NES_INIT
	sei			;disable IRQs
	cld			;decimal mode not supported
	ldx #$ff
	txs			;set up stack pointer
	inx			;increment X to 0
	stx PPU_MASK		;disable rendering
	stx DMC_FREQ		;disable DMC interrupts
	stx PPU_CTRL		;disable NMI interrupts
	bit PPU_STATUS		;clear VBL flag
	bit APU_CHAN_CTRL	;ack DMC IRQ bit 7
	lda #$40
	sta APU_FRAME		;disable APU Frame IRQ
	lda #$0F
	sta APU_CHAN_CTRL	;disable DMC, enable/init other channels.        
	ENDM


	MAC BLIT
.COUNT SET {1}
	REPEAT .COUNT
		bit $2002
	REPEND
	ENDM


	MAC INC_X
.COUNT SET {1}
	REPEAT .COUNT
		inx
	REPEND
	ENDM


	MAC INC_Y
.COUNT SET {1}
	REPEAT .COUNT
		iny
	REPEND
	ENDM


	MAC NOPS
.COUNT SET {1}
	REPEAT .COUNT
		nop
	REPEND
	ENDM

        
	MAC PPU_ADDR_SET
	; set 16bit address pointer in PPU
	lda #>{1}	; upper byte
	sta PPU_ADDR
	lda #<{1}	; lower byte
	sta PPU_ADDR
	ENDM


	MAC PPU_DECIMAL_00
	asl
	tax
	lda decimal_99_text_offset_80,x
	sta PPU_DATA
	inx
	lda decimal_99_text_offset_80,x
	sta PPU_DATA
	ENDM


	MAC PPU_DECIMAL_X0
	asl
	tax
	lda decimal_x9_text_offset_80,x
	sta PPU_DATA
	inx
	lda decimal_x9_text_offset_80,x
	sta PPU_DATA
	ENDM


	MAC PPU_FILL
	; unwound loop
	; sends {1} to PPU data {2} times
	REPEAT {2}
		lda #{1}
		sta PPU_DATA
	REPEND
	ENDM


	MAC PPU_LOOP
	; load {1} into PPU {2} times
	lda #{1}
	ldx #{2}
.loop
	sta PPU_DATA
	dex
	bne .loop
	ENDM


	MAC PPU_POPSLIDE
	; popslide {1} times
.COUNT	SET {1}
	REPEAT .COUNT
		pla
		sta PPU_DATA
	REPEND
	ENDM


	MAC PPU_PLOT
	; unwound loop
	; reads {2} bytes to PPU starting at {1}
.COUNT SET 0
	REPEAT {2}
		lda #{1}+.COUNT
		sta PPU_DATA
.COUNT SET .COUNT+1
	REPEND
	ENDM
        

	MAC PPU_PLOT_TEXT
	; write string at {2} to PPU at {1}
	; 00 terminated
	PPU_SETADDR {1}
	ldx #$00
.text_loop
	lda {2},x
	beq .text_done
	sta PPU_DATA
	inx
	jmp .text_loop
.text_done
	ENDM


	MAC PPU_SETVALUE
	; feed {1} to PPU
	lda #{1}
	sta PPU_DATA
	ENDM


	MAC SHIFT_L
.COUNT SET {1}
	REPEAT .COUNT
		asl
	REPEND
	ENDM


	MAC SHIFT_R
.COUNT SET {1}
	REPEAT .COUNT
		lsr
	REPEND
	ENDM

        
	MAC STATE_REGISTERS_RESET
	lda #$00
	sta state00
	sta state01
	sta state02
	sta state03
	sta state04
	sta state05
	sta state06
	sta state07
	sta scroll_x
	sta scroll_y
	ENDM


	MAC SAVE_REGS
	pha
	txa
	pha
	tya
	pha
	ENDM


	MAC RESTORE_REGS
	pla
	tay
	pla
	tax
	pla
	ENDM


;-------------------------------------------------------------------------------
; SLEEP clockcycles
; Original author: Thomas Jentzsch
; Inserts code which takes the specified number of cycles to execute.  This is
; useful for code where precise timing is required.
; LEGAL OPCODE VERSION MAY AFFECT FLAGS (uses 'bit' opcode)

NO_ILLEGAL_OPCODES EQM 1

	MAC SLEEP            ;usage: SLEEP n (n>1)
.CYCLES     SET {1}
	IF .CYCLES < 2
		ECHO "MACRO ERROR: 'SLEEP': Duration must be > 1"
		ERR
	ENDIF
	IF .CYCLES & 1
		IFNCONST NO_ILLEGAL_OPCODES
			nop 0
		ELSE
			bit $00
		ENDIF
		.CYCLES SET .CYCLES - 3
	ENDIF
	REPEAT .CYCLES / 2
		nop
	REPEND
	ENDM
