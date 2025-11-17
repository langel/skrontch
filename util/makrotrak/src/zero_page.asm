;;;;; VARIABLES

	seg.u ZEROPAGE
	org $0
        
wtf                    byte
nmi_lockout            byte ; *
temp00                 byte
temp01                 byte
temp02                 byte
temp03                 byte
temp04                 byte
temp05                 byte
temp06                 byte
temp07                 byte
state00                byte
state01                byte
state02                byte
state03                byte
state04                byte
state05                byte
state06                byte
state07                byte
rng0                   byte
oam_disable            byte ; *
controls               byte
controls_d             byte
ppu_mask_emph          byte ; *

state_jmp_lo   byte
state_jmp_hi   byte

timer_lo byte
timer_hi byte


scroll_x       byte
scroll_y       byte
scroll_nm 	   byte ; nametable

ent_slot            byte
ent_slot_start      byte
ent_spr_ptr         byte

collision_0_x	byte
collision_0_y	byte
collision_0_w	byte
collision_0_h	byte
collision_1_x	byte
collision_1_y	byte
collision_1_w	byte
collision_1_h	byte
collis_char_x  byte
collis_char_y  byte

arctang_velocity_lo  byte ; *
arctang_velocity_hi  byte ; *

rng_seed0  byte
rng_seed1  byte
rng_val0   byte
rng_val1   byte


spr_a          EQM $0202
spr_p          EQM $0201
spr_x          EQM $0203
spr_y          EQM $0200


	org $0400
ent_type  byte
	org $0420
ent_spawn byte
	org $0440
ent_x_hi  byte
	org $0460
ent_x     byte
	org $0480
ent_x_lo  byte
	org $04a0
ent_y     byte
	org $04c0
ent_y_lo  byte
	org $04e0
ent_h     byte
	org $0500
ent_h_lo  byte
	org $0520
ent_v     byte
	org $0540
ent_v_lo  byte
	org $0560
ent_dir   byte
	org $0580
ent_hp    byte
	org $05a0
ent_dmg   byte
	org $05c0
ent_hit   byte
	org $05e0
ent_r0    byte
	org $0600
ent_r1    byte
	org $0620
ent_r2    byte
	org $0640
ent_r3    byte
	org $0660
ent_r4    byte
	org $0680
ent_r5    byte
	org $06a0
ent_r6    byte
	org $06c0
ent_r7    byte
	org $06e0
ent_r8    byte
