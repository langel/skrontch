
const process_chr = (data) => {
	console.log('processing chr data ::');
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
	console.log(data.length);
	console.log((data.length >> 4) + ' tiles loading . .  .');

}

const chr_generate_canvas = (data) => {
	console.log(data);
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
	let scale = 2;
	can.style.width = (can.width * scale) + 'px';
	can.style.height = (can.height * scale) + 'px';
	return can;
	
}
