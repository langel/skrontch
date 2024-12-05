
const nes_pal_gen = () => {
}

const nes_pal_curr_canvas = (pals) => {
	let can = elem_new('canvas');
	can.width = 256;
	can.height = 16;
	let con = cancon(can);
	for (const [i, pal] of pals.entries()) {
		if (i % 4) {
			con.fillStyle = nes_pal_rgb[pal];
			con.fillRect((i << 4), 0, 16, 16);
		}
	}
	return can;
}

const nes_pal_full_canvas = () => {
	let can = elem_new('canvas');
	can.width = 256;
	can.height = 64;
	let con = cancon(can);
	for (const [i, color] of nes_pal_rgb.entries()) {
		con.fillStyle = color;
		con.fillRect((i % 16) << 4, (i >> 4) << 4, 16, 16);
	}
	return can;
}

let nes_pal_full;
let nes_pal_curr;
let nes_pal_edit;
let nes_pal_temp = 0;
let nes_pal_clicked = 0;


const nes_pal_editor_init = () => {

	nes_pal_edit = elem_get('nes_pal_edit');

	nes_pal_curr = elem_get('nes_pal_curr');
	nes_pal_curr.replaceChildren(nes_pal_curr_canvas(animation.palette));
	nes_pal_curr.addEventListener('click', (e) => {
		const rect = nes_pal_curr.getBoundingClientRect();
		const x = e.clientX - rect.left;
		const y = e.clientY - rect.top;
		nes_pal_clicked = x >> 4;
		nes_pal_full.style.display = 'block';
		nes_pal_edit.addEventListener('mouseleave', (e) => {
			nes_pal_full.style.display = 'none';
		});
	});
	nes_pal_curr.addEventListener('mousemove', (e) => {
		const rect = nes_pal_curr.getBoundingClientRect();
		const addr = (e.clientX - rect.left) >> 4;
		let span = elem_get('pattern_id_display');
		span.innerText = '$3f'+tohex(addr);
	});
	nes_pal_curr.addEventListener('mouseout', () => {
		let span = elem_get('pattern_id_display');
		span.innerText = '';
	});

	nes_pal_full = elem_get('nes_pal_full');
	nes_pal_full.replaceChildren(nes_pal_full_canvas());
	nes_pal_full.style.display = 'none';
	nes_pal_full.addEventListener('click', (e) => {
		animation.palette[nes_pal_clicked] = nes_pal_temp;
		let can = nes_pal_curr.children[0];
		let con = cancon(can);
		con.fillStyle = nes_pal_rgb[nes_pal_temp];
		con.fillRect(nes_pal_clicked << 4, 0, 16, 16);
		skrontch_update();
		let span = elem_get('pattern_id_display');
		span.innerText = '';
		nes_pal_full.style.display = 'none';
	});
	nes_pal_full.addEventListener('mousemove', (e) => {
		const rect = nes_pal_full.getBoundingClientRect();
		const x = e.clientX - rect.left;
		const y = e.clientY - rect.top;
		const color_id = (x >> 4) + ((y >> 4) << 4);
		nes_pal_temp = color_id;
		let span = elem_get('pattern_id_display');
		span.innerText = '0x'+tohex(color_id);
	});
	nes_pal_full.addEventListener('mouseout', () => {
		let span = elem_get('pattern_id_display');
		span.innerText = '';
	});
}
