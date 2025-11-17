ent_nothing_id           eqm $00
ent_ball_id              eqm $01
ent_laser_id             eqm $02


ent_spawn_lo:
	byte <do_nothing
	byte <ent_ball_spawn
	byte <ent_laser_spawn
ent_spawn_hi:
	byte >do_nothing
	byte >ent_ball_spawn
	byte >ent_laser_spawn

ent_update_lo:
	byte <do_nothing
	byte <ent_ball_update
	byte <ent_laser_update
ent_update_hi:
	byte >do_nothing
	byte >ent_ball_update
	byte >ent_laser_update
