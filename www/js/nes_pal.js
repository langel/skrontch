
const nes_pal_gen = () => {
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
	can.addEventListener('mousemove', (e) => {
		const rect = can.getBoundingClientRect();
		const x = e.clientX - rect.left;
		const y = e.clientY - rect.top;
		// XXX should track scale of can elesewhere and divide by
		const color_id = (x >> 4) + ((y >> 4) << 4);
		let span = elem_get('pattern_id_display');
		span.innerText = '0x'+tohex(color_id);
	});
	can.addEventListener('mouseout', () => {
		let span = elem_get('pattern_id_display');
		span.innerText = '';
	});
	return can;
}
