

const blank = "&#x2800;";
const br = "<br>";

// value convertors
const char = (ord) => String.fromCharCode(ord);
const ord = (char) => char.charCodeAt(0);
const tohex = (x) => x.toString(16).padStart(2, '0'); 
const isset = (x) => (typeof x !== 'undefined');

const obj_clone = (obj) => JSON.parse(JSON.stringify(obj));

// DOM
const delay = (ms) => { return new Promise(resolve => setTimeout(resolve, ms)); }
const elem_new = (t) => document.createElement(t);
const elem_get = (t) => document.getElementById(t);
const frame_next = () => { return new Promise(resolve => requestAnimationFrame(resolve)); }
const tobottom = () => window.scrollTo(0, document.body.scrollHeight);

// local storage
const localget = (key) => localStorage.getItem(key);
const localset = (key, val) => localStorage.setItem(key, val);
const force_donload = (data, filename) => {
	const blob = new Blob([data]);
	const url = URL.createObjectURL(blob);
	const link = document.createElement('a');
	link.href = url;
	link.download = filename;
	link.click();
	URL.revokeObjectURL(url);
}

// canvas
const cancon = (can) => can.getContext('2d');
const canvas_clone = (canvas) => {
	let can = document.createElement('canvas');
	can.width = canvas.width;
	can.height = canvas.height;
	can.style.marginBottom = '2px';
	let con = can.getContext('2d');
	con.drawImage(canvas, 0, 0);
	return { can: can, con: con };
}
const canvas_from_img = (img) => {
	let can = document.createElement('canvas');
	can.width = img.width;
	can.height = img.height;
	can.style.marginBottom = '2px';
	let con = can.getContext('2d');
	con.drawImage(img, 0, 0);
	return { can: can, con: con };
}
const canvas_scale = (can, scale) => {
	can.style.width = (can.width * scale) + 'px';
	can.style.height = (can.height * scale) + 'px';
}


// XXX uh need another file for nes specific stuff
const nes_find_color_id = (color) => {
	let color_id = nes_palette.indexOf(color);
	// make sure its the correct black!!
	if (color_id == 0x0d) color_id = 0x0f;
	return color_id;
}
const pattern_from_grays = async (data) => { // async?
	// given canvas context 16x16 pixel data, creates nes pattern data
	let chr = new Uint8Array(16);
	for (p = 0; p < 256; p += 4) {
		let val = nes_gvals.indexOf(data.data[p]);
		if (val == -1) val = 0;
		// target = 64 pixels * 2bpp
		let x = 1 << (7 - ((p >> 2) % 8));
		let y = p >> 5;
		chr[y] |= (val & 0x01) ? x : 0;
		chr[y+8] |= (val & 0x02) ? x : 0;
	}
	return chr;
}
const pixels_from_image = async (img, x, y, w, h) => {
	let canvas = document.createElement('canvas');
	let context = canvas.getContext('2d');
	canvas.width = w;
	canvas.height = h;
	context.drawImage(img, x, y, w, h, 0, 0, w, h);
	return context.getImageData(0, 0, w, h).data;
}
const pattern_pos = function(tile_id) {
	let tileset_width = 128;
	let tile_w = 8;
	return { x: (tile_id % tile_w) << 3, y: (tile_id >> 4) << 3 };
}

// arrays
// XXX should move nes bank specific codes to another file
const array_blank_pages = (arr, val) => {
	// arr = array to be examined
	// val = empty value to seeach for
	// return array of empty pages
	let out = [];
	let page_blank = true;
	let page_id = 0;
	for (let i = 0; i < arr.length; i++) {
		if (arr[i] != val) page_blank = false;
		if (i % 256 == 0 && i != 0) {
			if (page_blank == true) out.push(page_id);
			page_id++;
			page_blank = true;
		}
	}
	// include final page if unfilled
	if (page_blank == true) out.push(page_id);
	return out;
}
const array_common = (arrs) => { 
	return arrs[0].filter(val => arrs.every(arr => arr.includes(val)));
}
const array_value_segments = (arr, val, min) => {
	// arr = array to be examined
	// val = segment value to search for
	// min = minimum integer size of segment
	let val_count = 0;
	let start_index = -1;
	let segments = [];
	for (let i = 0; i < arr.length; i++) {
		if (arr[i] === val) {
			if (val_count === 0) {
				start_index = i;
			}
			val_count++;
		}
		else {
			if (val_count >= min) {
				segments.push({ 
					start: start_index, 
					end: i - 1,
					length: i - 1 - start_index,
				});
			}
			val_count = 0;
		}
	}
	if (val_count >= min) {
		segments.push({ 
			start: start_index, 
			end: arr.length - 1,
			length: arr.length - 1 - start_index,
		});
	}
	return segments;
}
