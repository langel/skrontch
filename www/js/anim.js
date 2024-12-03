
/*

TO DO --

	invent skrontch project json object
	skorntch project export/import
	new animation button / naming field
	edit number of sprites and steps
	update animation json data onchange
	generate assembly
	preview animation
	palette manager

*/


let sprite_count = 2;
let step_count = 4;
let anim_form;

let anim_fps = 5;
let anim_last = 0;
let anim_counter = 0;

let anim_play_status = true;

const anim_schema = {
	chr: '',
	fps: 8,
	steps: [],
};
const anim_step_schema = {
	frame_length: 8,
	sprites: [],
}
const anim_sprite_schema = {
	's': null,
	'a': null,
	'x': null,
	'y': null,
};

const anim_sprite_defs = {
	's': 'char index',
	'a': 'attributes',
	'x': 'x offset',
	'y': 'y offset',
};

const anim_fps_rates = [ 60, 30, 20, 15, 12, 10, 8, 6, 5, 4, 3, 2, 1 ];

const anim_init = () => {
	// fresh session setup default animations
	if (Object.entries(proj.anim.anims) == 0) {
		let new_anim = obj_clone(anim_schema);
		for (let i = 0; i < 4; i++) {
			let step = obj_clone(anim_step_schema);
			for (let j = 0; j < 2; j++) {
				let sprite = obj_clone(anim_sprite_schema);
				step.sprites.push(sprite);
			}
			new_anim.steps.push(step);
		}
		const defname = 'New Default';
		proj.anim.anims[defname] = new_anim;
		proj.anim.current = defname;
	}
	animation = proj.anim.anims[proj.anim.current];
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
	anim_fps = animation.fps;
	slider.value = anim_fps_rates.indexOf(proj.anim.fps);
	slider.dispatchEvent(new Event('input'));
	// setup animation controls
	elem_get('anim_step_back').addEventListener('click', () => {
		anim_play_status = false;
		anim_counter--;
	});
	elem_get('anim_play').addEventListener('click', () => anim_play_status = true);
	elem_get('anim_pause').addEventListener('click', () => anim_play_status = false);
	elem_get('anim_step_forward').addEventListener('click', () => {
		anim_play_status = false;
		anim_counter++;
	});
	// construct data form
	anim_render_animation_form();
	// set anim cycle in motion
	anim_update();
	// generate chr view
	anim_render_chr_select();
	// generate nes full palette
	let pal_full_canvas = nes_pal_full_canvas();
	let pal_full = elem_get('pal_full');
	pal_full.appendChild(pal_full_canvas);
}

const anim_process = () => {
	let table = elem_get('anim_table');
}

const anim_update = () => {
	if (anim_play_status) {
		let then = anim_last;
		let now = window.performance.now();
		let elapsed = now - then;
		let fps_rate = 1000 / anim_fps;
		if (elapsed > fps_rate) {
			anim_counter++;
			anim_last = now - (elapsed % fps_rate);
		}
	}
	let div = elem_get('anim_counter');
	div.innerHTML = anim_counter;
	anim_render_frame();
	requestAnimationFrame(anim_update);
}


const anim_render_chr = (chr) => {
	if (chr == 'null') return;
	let canvas = chr_generate_canvas(proj.chr[chr]);
	canvas.setAttribute('id', 'chr_view');
	canvas.addEventListener('mousemove', (e) => {
		const rect = canvas.getBoundingClientRect();
		const x = e.clientX - rect.left;
		const y = e.clientY - rect.top;
		// XXX should track scale of canvas elesewhere and divide by
		const pattern_id = (x >> 4) + ((y >> 4) << 4);
		let span = elem_get('pattern_id_display');
		span.innerText = '0x'+tohex(pattern_id);
	});
	canvas.addEventListener('mouseout', () => {
		let span = elem_get('pattern_id_display');
		span.innerText = '';
	});
	let chr_holder = elem_get('chr_holder');
	chr_holder.innerHTML = '';
	chr_holder.appendChild(canvas);
}


const anim_chr_new = (name) => {
	animation.chr = name;
	anim_render_chr_select();
}

const anim_render_chr_select = () => {
	let chr_select = elem_get('chr_bank_selector');
	chr_select.innerHTML = '';
	if (Object.keys(proj.chr).length == 0) {
		let option = elem_new('option');
		option.value = 'null';
		option.innerText = 'drag .chr files in';
		option.setAttribute('selected', true);
		chr_select.appendChild(option);
		chr_select.selectedIndex = 0;
	}
	else for (const name of Object.keys(proj.chr).sort()) {
		let option = elem_new('option');
		option.value = name;
		option.innerText = name;
		chr_select.appendChild(option);
	}
	chr_select.onchange = () => { 
		animation.chr = chr_select.value;
		anim_render_chr(chr_select.value); 
		skrontch_update();
	}
	if (animation.chr != '') {
		chr_select.value = animation.chr;
		chr_select.dispatchEvent(new Event('change'));
	}
}


const anim_render_frame = () => {
	// draw preview
	let src = elem_get('chr_view');
	if (src == null) return;
	let can = chr_gen_sprite(proj.chr[animation.chr], anim_counter % 256);
	canvas_scale(can, 16);
	let preview = elem_get('anim_preview');
	preview.innerHTML = '';
	preview.appendChild(can);
	// highlight step
	let step_rows = document.getElementsByClassName('anim_step_row');
	let step_count = step_rows.length;
	for (let i = 0; i < step_count; i++) {
		step_rows[i].classList.remove('highlight');
		if (i == anim_counter % step_count) {
			step_rows[i].classList.add('highlight');
		}
	}
}


anim_render_animation_form = async () => {
	anim_form = elem_get('form');
	let table = elem_new('table');
	table.setAttribute('id', 'anim_table');
	table.setAttribute('width', '100%');
	let header = elem_new('tr');
	header.innerHTML = '<td></td>';
	for (let cols = 0; cols < sprite_count; cols++) {
		header.innerHTML += '<td>Sprite ' + tohex(cols) + '</td>';
	}
	table.appendChild(header);
	// create form fields
	for (const [rows, step] of animation.steps.entries()) {
		let row = elem_new('tr');
		row.classList.add('anim_step_row');
		let row_meta = elem_new('td');
		row_meta.innerHTML = 'Anim Step ' + tohex(rows);
		row.appendChild(row_meta);
		for (const [cols, sprite] of step.sprites.entries()) {
			let cell = elem_new('td');
			for (const [attr, nick] of Object.entries(anim_sprite_defs)) {
				cell.innerHTML += attr+' ';
				let input = elem_new('input');
				cell.appendChild(input);
				input.id = 'anim_'+rows+'_'+cols+'_'+attr;
	//			input.setAttribute('type', 'number');
				input.placeholder = nick;
				input.setAttribute('step_attr', rows);
				input.setAttribute('sprite_attr', cols);
				input.setAttribute('attr_attr', attr);
				console.log(input.value);
				cell.innerHTML += '<br>';
			}
			row.appendChild(cell);
		}
		table.appendChild(row);
	}
	anim_form.appendChild(table);
	// populate form fields
	for (const [rows, step] of animation.steps.entries()) {
		for (const [cols, sprite] of step.sprites.entries()) {
			for (const [attr, nick] of Object.entries(anim_sprite_defs)) {
				let input = elem_get('anim_'+rows+'_'+cols+'_'+attr);
				let val = animation.steps[rows].sprites[cols][attr];
				if (val !== null) input.value = parseInt(val);
				input.addEventListener('input', (e) => {
					console.log('ass');
					let step = input.getAttribute('step_attr');
					let sprite = input.getAttribute('sprite_attr');
					let attr = input.getAttribute('attr_attr');
					let val = e.target.value;
					animation.steps[step].sprites[sprite][attr] = val;
					console.log(val);
				console.log(input);
					skrontch_update();
				});
			}
		}
	}
}
