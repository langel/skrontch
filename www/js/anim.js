
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

let anim_fps = 5;
let anim_last = 0;
let anim_counter = 0;

const anim_step_schema = {
	's': 'char index',
	'a': 'attributes',
	'x': 'x offset',
	'y': 'y offset',
};

const anim_fps_rates = [ 60, 30, 20, 15, 12, 10, 8, 6, 5, 4, 3, 2, 1 ];

const anim_init = () => {
	// setup anim preview fps slider
	let slider = elem_get('fps_rate_slider');
	slider.setAttribute('max', anim_fps_rates.length - 1);
	slider.oninput = () => {
		anim_fps = anim_fps_rates[slider.value];
		let span = elem_get('fps_rate_display');
		span.innerHTML = anim_fps + ' fps';
		proj.anim.fps = anim_fps;
		skrontch_update();
	}
	anim_fps = anim_fps_rates[proj.anim.fps];
	slider.value = anim_fps_rates.indexOf(proj.anim.fps);
	slider.dispatchEvent(new Event('input'));
	// construct data form
	form = elem_get('form');
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
	// set anim cycle in motion
	anim_update();
	// generate chr view
	let canvas = chr_generate_canvas(proj.chr["char_4_theo.chr"]);
	console.log(canvas);
	let chr = elem_get('chr_holder');
	console.log(chr);
	chr.appendChild(canvas);
	let chr_select = elem_get('chr_bank_selector');
	for (const name of Object.keys(proj.chr)) {
		let option = elem_new('option');
		option.value = name;
		option.innerText = name;
		chr_select.appendChild(option);
	}
}

const anim_process = () => {
	let table = elem_get('anim_table');
}

const anim_update = () => {
	let then = anim_last;
	let now = window.performance.now();
	let elapsed = now - then;
	let fps_rate = 1000 / anim_fps;
	if (elapsed > fps_rate) {
		anim_counter++;
		anim_last = now - (elapsed % fps_rate);
		let div = elem_get('counter');
		div.innerHTML = anim_counter;
	}
	requestAnimationFrame(anim_update);
}



