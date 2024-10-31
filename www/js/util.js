

const blank = "&#x2800;";
const br = "<br>";

// value convertors
const char = (ord) => String.fromCharCode(ord);
const ord = (char) => char.charCodeAt(0);
const tohex = (x) => x.toString(16).padStart(2, '0'); 

// DOM
const element_new = (t) => document.createElement(t);
const frame_next = () => { return new Promise(resolve => requestAnimationFrame(resolve)); }
const tobottom = () => window.scrollTo(0, document.body.scrollHeight);

// local storage
const localget = (key) => localStorage.getItem(key);
const localset = (key, val) => localStorage.setItem(key, val);

// arrays
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
