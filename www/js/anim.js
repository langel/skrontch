
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
	palette: [
		0, 0x01, 0x12, 0x23,
		0, 0x04, 0x15, 0x26,
		0, 0x09, 0x19, 0x29,
		0, 0x0c, 0x1c, 0x2c,
	],
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
	's': 'index',
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
	// menu bar
	anim_menubar_init();
	// construct data form
	anim_render_animation_form();
	// set anim cycle in motion
	anim_update();
	// generate chr view
	anim_render_chr_select();
	// generate nes palette editor
	nes_pal_editor_init();
	// export tables
	elem_listen('anim_tables_export', 'click', (e) => {
		e.preventDefault();
		let out_s = "char_missile_theo_spr:"
		let out_a = "char_missile_theo_att:"
		let out_x = "char_missile_theo_x:"
		let out_y = "char_missile_theo_y:"
		let anims = Object.entries(proj.anim.anims).sort();
		for (const [name, data] of anims) {
			out_s += "\n\thex";
			out_a += "\n\thex";
			out_x += "\n\thex";
			out_y += "\n\thex";
			for (const [i, step] of data.steps.entries()) {
				for (const sprite of step.sprites) {
					out_s += ' '+tohex(sprite.s);
					out_a += ' '+tohex(sprite.a);
					out_x += ' '+tohex(sprite.x);
					out_y += ' '+tohex(sprite.y);
				}
			}
		}
		let output = out_s + "\n" + out_a + "\n" + out_x + "\n" + out_y + "\n";
		elem_get('anim_output').innerText = output;
	});
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

// renders whole graphical banks
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
	let step = animation.steps[anim_counter % animation.steps.length];
	let canvas = elem_new('canvas');
	canvas.width = 64;
	canvas.height = 32;
	let ctx = cancon(canvas);
	// XXX should calculate size/pos of all frames and sprites
	//     then we can draw muted 0,0 lines and center everything 
	//let bounds = { x1:0, y1:0, x2:0, y2:0 };
	for (const sprite of step.sprites) {
		if (sprite.s !== null && sprite.a !== null && sprite.x !== null && sprite.y !== null) {
			let first = (sprite.a & 0x03) << 2;
			let colors = animation.palette.slice(first, first+4);
			let can = chr_gen_sprite(proj.chr[animation.chr], sprite.s, sprite.a, colors);
			ctx.drawImage(can, parseInt(sprite.x)+24, parseInt(sprite.y)+8);
		}
	}
	canvas_scale(canvas, 4);
	let preview = elem_get('anim_preview');
	preview.innerHTML = '';
	preview.appendChild(canvas);
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


// XXX needs proper abstraction
//     very redundant processes
const anim_menubar_init = () => {
	// construct animation selector
	anim_menubar_build_animation_select();
	elem_listen('animation_select', 'input', (e) => {
		proj.anim.current = e.target.value;
		animation = proj.anim.anims[proj.anim.current];
		requestAnimationFrame(() => {
			anim_render_animation_form();
		});
	});
	// rename
	elem_listen('anim_rename', 'click', (e) => {
		e.preventDefault();
		elem_get('anim_menu').style.display = 'none';
		elem_get('anim_rename_form').style.display = 'block';
		elem_get('anim_rename_error').style.display = 'none';
		let input = elem_get('anim_rename_input');
		input.value = proj.anim.current;
		requestAnimationFrame(() => {
			input.focus()
			input.dispatchEvent(new Event('input'));
			input.setSelectionRange(input.value.length, input.value.length);
		});
	});
	elem_listen('anim_rename_input', 'input', (e) => {
		let input = elem_get('anim_rename_input');
		let val = input.value;
		for (const [title, data] of Object.entries(proj.anim.anims)) {
			if (title == val) {
				// display error
				input.classList.add('error');
				elem_get('anim_rename_error').style.display = 'inline';
				elem_get('anim_rename_error').innerText = 'Error: Name already in use.';
				return;
			}
		}
		input.classList.remove('error');
		elem_get('anim_rename_error').style.display = 'none';
	});
	elem_listen('anim_rename_input', 'keydown', (e) => {
		if (e.key == 'Enter') elem_get('anim_rename_save').click();
	});
	elem_listen('anim_rename_save', 'click', (e) => {
		e.preventDefault(); 
		let input = elem_get('anim_rename_input');
		if (input.classList.contains('error')) return;
		proj.anim.anims[input.value] = obj_clone(proj.anim.anims[proj.anim.current]);
		delete proj.anim.anims[proj.anim.current];
		proj.anim.current = input.value;
		elem_get('anim_menu').style.display = 'block';
		elem_get('anim_rename_form').style.display = 'none';
		anim_menubar_build_animation_select();
		skrontch_update();
	});
	elem_listen('anim_rename_cancel', 'click', (e) => {
		e.preventDefault(); 
		elem_get('anim_menu').style.display = 'block';
		elem_get('anim_rename_form').style.display = 'none';
	});
	// clone
	elem_listen('anim_clone', 'click', (e) => {
		e.preventDefault();
		elem_get('anim_menu').style.display = 'none';
		elem_get('anim_clone_form').style.display = 'block';
		elem_get('anim_clone_error').style.display = 'none';
		let input = elem_get('anim_clone_input');
		input.value = proj.anim.current;
		requestAnimationFrame(() => {
			input.focus()
			input.dispatchEvent(new Event('input'));
			input.setSelectionRange(input.value.length, input.value.length);
		});
	});
	elem_listen('anim_clone_input', 'input', (e) => {
		let input = elem_get('anim_clone_input');
		let val = input.value;
		for (const [title, data] of Object.entries(proj.anim.anims)) {
			if (title == val) {
				// display error
				input.classList.add('error');
				elem_get('anim_clone_error').style.display = 'inline';
				elem_get('anim_clone_error').innerText = 'Error: Name already in use.';
				return;
			}
		}
		input.classList.remove('error');
		elem_get('anim_clone_error').style.display = 'none';
	});
	elem_listen('anim_clone_input', 'keydown', (e) => {
		if (e.key == 'Enter') elem_get('anim_clone_save').click();
	});
	elem_listen('anim_clone_save', 'click', (e) => {
		e.preventDefault(); 
		let input = elem_get('anim_clone_input');
		if (input.classList.contains('error')) return;
		proj.anim.anims[input.value] = obj_clone(proj.anim.anims[proj.anim.current]);
		proj.anim.current = input.value;
		animation = proj.anim.anims[proj.anim.current];
		elem_get('anim_menu').style.display = 'block';
		elem_get('anim_clone_form').style.display = 'none';
		anim_menubar_build_animation_select();
		skrontch_update();
	});
	elem_listen('anim_clone_cancel', 'click', (e) => {
		e.preventDefault(); 
		elem_get('anim_menu').style.display = 'block';
		elem_get('anim_clone_form').style.display = 'none';
	});
	// delete
	elem_listen('anim_delete', 'click', (e) => {
		e.preventDefault();
		elem_get('anim_delete_name').innerText = proj.anim.current;
		elem_get('anim_menu').style.display = 'none';
		elem_get('anim_delete_form').style.display = 'block';
		elem_get('anim_delete_error').style.display = 'none';
	});
	elem_listen('anim_delete_save', 'click', (e) => {
		e.preventDefault(); 
		let select = elem_get('animation_select');
		if (select.options.length > 1) {
			let index = select.selectedIndex;
			delete proj.anim.anims[select.value];
			select.remove(index);
			select.selectedIndex = 0;
			proj.anim.current = select.children[0].value;
			animation = proj.anim.anims[proj.anim.current];
			elem_get('anim_menu').style.display = 'block';
			elem_get('anim_delete_form').style.display = 'none';
			anim_menubar_build_animation_select();
			skrontch_update();
		}
	});
	elem_listen('anim_delete_cancel', 'click', (e) => {
		e.preventDefault(); 
		elem_get('anim_menu').style.display = 'block';
		elem_get('anim_delete_form').style.display = 'none';
	});
}

const anim_menubar_build_animation_select = () => {
	let anim_select = elem_get('animation_select');
	anim_select.innerHTML = '';
	for (const [name, data] of Object.entries(proj.anim.anims)) {
		let option = elem_new('option');
		option.value = name;
		option.innerText = name;
		if (option.value == proj.anim.current) {
			option.setAttribute('selected', true);
		}
		anim_select.appendChild(option);
	}
}


const anim_render_animation_form = async () => {
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
				let input_dec = elem_new('input');
				input_dec.id = 'anim_'+rows+'_'+cols+'_'+attr;
				input_dec.setAttribute('step_attr', rows);
				input_dec.setAttribute('sprite_attr', cols);
				input_dec.setAttribute('attr_attr', attr);
				let input_hex = input_dec.cloneNode();
				input_dec.id += '_dec';
				input_dec.type = 'number';
				input_dec.placeholder = nick;
				input_dec.classList.add('dec_in');
				input_hex.id += '_hex';
				input_hex.setAttribute('maxlength', 2);
				input_hex.placeholder = 'hex';
				input_hex.classList.add('hex_in');
				cell.innerHTML += attr+' ';
				cell.appendChild(input_dec);
				cell.appendChild(input_hex);
				cell.innerHTML += '<br>';
			}
			row.appendChild(cell);
		}
		table.appendChild(row);
	}
	anim_form.replaceChildren(table);
	// populate form fields
	for (const [rows, step] of animation.steps.entries()) {
		for (const [cols, sprite] of step.sprites.entries()) {
			for (const [attr, nick] of Object.entries(anim_sprite_defs)) {
				let input_dec = elem_get('anim_'+rows+'_'+cols+'_'+attr+'_dec');
				let input_hex = elem_get('anim_'+rows+'_'+cols+'_'+attr+'_hex');
				let val = animation.steps[rows].sprites[cols][attr];
				if (val !== null && val !== undefined) {
					input_dec.value = parseInt(val);
					input_hex.value = tohex(parseInt(val));
				}
				input_dec.addEventListener('input', (e) => {
					const input = e.target;
					let step = input.getAttribute('step_attr');
					let sprite = input.getAttribute('sprite_attr');
					let attr = input.getAttribute('attr_attr');
					let val = input.value;
					if (val > 255) val = 0;
					else if (val < 0) val = 255;
					input.value = val;
					let input_hex = elem_get('anim_'+step+'_'+sprite+'_'+attr+'_hex');
					input_hex.value = tohex(parseInt(val));
					animation.steps[step].sprites[sprite][attr] = val;
					skrontch_update();
				});
				input_hex.addEventListener('focus', (e) => {
					if (e.target.value == 'NaN') e.target.select();
				});
				input_hex.addEventListener('input', (e) => {
					const input = e.target;
					let step = input.getAttribute('step_attr');
					let sprite = input.getAttribute('sprite_attr');
					let attr = input.getAttribute('attr_attr');
					if (!/^[0-9A-Fa-f]+$/.test(input.value)) {
						input.classList.add('error');
						return;
					}
					let val = parseInt(input.value, 16);
					input.classList.remove('error');
					let input_dec = elem_get('anim_'+step+'_'+sprite+'_'+attr+'_dec');
					input_dec.value = val;
					animation.steps[step].sprites[sprite][attr] = val;
					skrontch_update();
				});
			}
		}
	}
}
