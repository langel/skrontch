
const process_chr = (data) => {
/* TO DO
	-- schema choices:
		2bpp 8x8   planar (nes)
		4bpp 8x8   planar (snes(all)/pce bg tiles)
		4bpp 8x8   linear (megadrive)
		4bpp 16x16 planar (pce sprites)
	-- draw to a canvas
	-- editing would be out of scope for this file
		should parallel update data and canvas
*/
	console.log((data.length >> 4) + ' tiles loading . .  .');

}

const chr_generate_canvas = (data) => {
	process_chr(data);
	let chr_count = data.length >> 4;
	let can = document.createElement("canvas");
	can.width = 128;
	can.height = chr_count >> 1;
	let con = can.getContext("2d", { willReadFrequently: true });
	// each character pattern
	for (let j = 0; j < chr_count; j++) {
		let x = (j % 16) << 3;
		let y = (j >> 4) << 3;
		let indirect = j << 4;
		// each row
		for (let k = 0; k < 8; k++) {
			let lo = data[indirect + k];
			let hi = data[indirect + k + 8];
			// each pixel
			for (let l = 7; l >= 0; l--) {
				let val = (lo & (1 << l)) ? 1 : 0;
				val |= (hi & (1 << l)) ? 2 : 0;
				val = nes_gvals[val];
				let fill_style = 'rgb(' + val + ' ' + val + ' ' + val + ')';
				con.fillStyle = fill_style;
				con.fillRect(x + 7 - l, y + k, 1, 1);
			}
		}
	}
	canvas_scale(can, 2);
	return can;
}

const chr_gen_sprite = (data, index, attr, pal) => {
	let mirror = attr >> 7;
	let flip = (attr >> 6) & 0x01;
	let can = elem_new('canvas');
	let con = cancon(can);
	can.width = 8;
	can.height = 8;
	let offset = index << 4;
	for (let k = 0; k < 8; k++) {
		let lo = data[offset + k];
		let hi = data[offset + k + 8];
		// each pixel
		for (let l = 7; l >= 0; l--) {
			let val = (lo & (1 << l)) ? 1 : 0;
			val |= (hi & (1 << l)) ? 2 : 0;
			if (val) {
				con.fillStyle = nes_pal_rgb[pal[val]];
				con.fillRect(7 - l, k, 1, 1);
			}
		}
	}
	return can;
}

const chr_gen_tile = (data, index) => {
	let can = elem_new('canvas');
	let con = cancon(can);
	can.width = 8;
	can.height = 8;
	let offset = index << 4;
	for (let k = 0; k < 8; k++) {
		let lo = data[offset + k];
		let hi = data[offset + k + 8];
		// each pixel
		for (let l = 7; l >= 0; l--) {
			let val = (lo & (1 << l)) ? 1 : 0;
			val |= (hi & (1 << l)) ? 2 : 0;
			let color = nes_palette[(val << 4) + 1];
			let fill_style = 'rgb(' + (color >> 16) + ' ' + ((color >> 8) & 0xff) + ' ' + (color & 0xff) + ')';

			con.fillStyle = fill_style;
			con.fillRect(7 - l, k, 1, 1);
		}
	}
	return can;
}
