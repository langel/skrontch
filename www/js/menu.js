

const menu_init = () => {
	let save = elem_get('control_save');
	save.addEventListener('click', (e) => {
		e.preventDefault();
		force_donload(JSON.stringify(proj, null, 3), 'project.skrontch');
	});
}
