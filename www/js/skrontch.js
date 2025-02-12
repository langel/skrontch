
let proj = {
	// animation settings
	anim: {
		fps: 8,
		anims: {},
	},
	// chr rom banks
	chr: {},
};



const skrontch_init = () => {
	// load project from local storage
	let local_proj = localget('skrontch_current_project');
	if (local_proj !== null) proj = JSON.parse(local_proj);
	// initialize subsystems
	anim_init();
	drop_init();
	menu_init();
	nes_pal_gen();
}



const skrontch_update = () => {
	localset('skrontch_current_project', JSON.stringify(proj));
}



window.addEventListener("DOMContentLoaded", () => {
	skrontch_init();
});
