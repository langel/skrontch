
const process_nsf = (data) => {
	logout('processing::');
	let hi, lo;
	// load address
	hi = tohex(data[9]);
	lo = tohex(data[8]);
	logout('$' + hi + lo + ' address load');
	// init address
	hi = tohex(data[11]);
	lo = tohex(data[10]);
	logout('$' + hi + lo + ' address init');
	// play adress
	hi = tohex(data[13]);
	lo = tohex(data[12]);
	logout('$' + hi + lo + ' address play');
	// credits
	let title = '';
	let artist = '';
	let copyright = '';
	for (let i = 0; i < 32; i++) {
		title += char(data[0x0e + i]);
		artist += char(data[0x02e + i]);
		copyright += char(data[0x4e + i]);
	}
	logout('title: ' + title);
	logout('artist: ' + artist);
	logout('copyright: ' + copyright);
}
