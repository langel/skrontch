
/*

TO DO --

	invent skrontch project json object
	skorntch project export/import
	new animation button / naming field
	edit number of sprites and steps
	update animation json data onchange
	generate assembly
	import/save/display chr files
	preview animation
	palette manager

*/


let sprite_count = 2;
let step_count = 4;
let form;

const anim_step_schema = {
	's': 'tile index',
	'a': 'attributes',
	'x': 'x offset',
	'y': 'y offset',
};

const anim_init = () => {
	let table = elem_new('table');
	table.setAttribute('id', 'anim_table');
	table.setAttribute('width', '100%');
	let header = elem_new('tr');
	header.innerHTML = '<td></td>';
	for (let cols = 0; cols < sprite_count; cols++) {
		header.innerHTML += '<td>Sprite ' + tohex(cols) + '</td>';
	}
	table.appendChild(header);
	for (let rows = 0; rows < step_count; rows++) {
		let row = elem_new('tr');
		let row_meta = elem_new('td');
		row_meta.innerHTML = 'Anim Step ' + tohex(rows);
		row.appendChild(row_meta);
		for (let cols = 0; cols < sprite_count; cols++) {
			let cell = elem_new('td');
	//		s_input.value = 0;
		// XXX build these inputs using above schema loop
			for (const [id, nick] of Object.entries(anim_step_schema)) {
				cell.innerHTML += id+' ';
				let input = elem_new('input');
				input.setAttribute('id', 'anim_'+rows+'_'+cols+'_'+id);
				input.setAttribute('placeholder', nick);
				input.setAttribute('type', 'number');
				input.setAttribute('step', '1');
				cell.appendChild(input);
				cell.innerHTML += '<br>';
			}
			row.appendChild(cell);
		}
		table.appendChild(row);
	}
	form.appendChild(table);
}

const anim_process = () => {
	let table = elem_get('anim_table');
}



window.addEventListener("DOMContentLoaded", () => {
	form = elem_get('form');
	anim_init();
});
