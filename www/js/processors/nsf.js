
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

	// CONVERTING NSF TO NES ROM
	// build nes rom header
	let nes_header = new Uint8Array(16);
	nes_header[0] = ord('N');
	nes_header[1] = ord('E');
	nes_header[2] = ord('S');
	nes_header[3] = 0x1a;
	nes_header[4] = 2; // (PRG 16k)
	nes_header[5] = 1; // (CHR 8k)
	for (let i = 6; i < 16; i++) nes_header[i] = 0;
	let nes_rom = new Uint8Array(16 + 32 * 1024);
	// copy nsf data to load target
}
