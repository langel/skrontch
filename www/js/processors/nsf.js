
const process_nsf = (data) => {
	logout('processing nsf data ::');
	let hi, lo;
	// load address
	let address_load = (data[9] << 8) + data[8];
	hi = tohex(data[9]);
	lo = tohex(data[8]);
	logout('$' + hi + lo + ' address load');
	console.log(address_load);
	// init address
	let address_init = (data[11] << 8) + data[10];
	hi = tohex(data[11]);
	lo = tohex(data[10]);
	logout('$' + hi + lo + ' address init');
	console.log(address_init);
	// play adress
	let address_play = (data[13] << 8) + data[12];
	hi = tohex(data[13]);
	lo = tohex(data[12]);
	logout('$' + hi + lo + ' address play');
	console.log(address_play);
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
	nes_rom.set(nes_header, 0);
	console.log(address_load - 0x8000);
	console.log(0x8000);
	console.log(data.length);
	nes_rom.set(data.slice(0x80), address_load - 0x8000);
	console.log(nes_rom);
/* TO DO
	-- find empty chunks of rom
	-- create portable booter binary
	-- set rom vectors
	-- force donload rom
	-- fail warning on oversized nsf
*/
}
