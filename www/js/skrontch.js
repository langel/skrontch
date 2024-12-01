
let proj = {
	anim: {
		fps: 8,
	},
};



const skrontch_init = () => {
	// load project from local storage
	let local_proj = localget('skrontch_current_project');
	if (local_proj !== null) proj = JSON.parse(local_proj);
	console.log(proj);
	// initialize subsystems
	anim_init();
}



const skrontch_update = () => {
	console.log(proj.toString());
	localset('skrontch_current_project', JSON.stringify(proj));
}



window.addEventListener("DOMContentLoaded", () => {
	skrontch_init();
});
