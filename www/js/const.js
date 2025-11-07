const nes_gvals = [0x11, 0x69, 0xa2, 0xeb];
const nes_grays = [
	new ImageData(new Uint8ClampedArray([nes_gvals[0], nes_gvals[0], nes_gvals[0], 0xff]), 1, 1),
	new ImageData(new Uint8ClampedArray([nes_gvals[1], nes_gvals[1], nes_gvals[1], 0xff]), 1, 1),
	new ImageData(new Uint8ClampedArray([nes_gvals[2], nes_gvals[2], nes_gvals[2], 0xff]), 1, 1),
	new ImageData(new Uint8ClampedArray([nes_gvals[3], nes_gvals[3], nes_gvals[3], 0xff]), 1, 1),
];

const nes_pal_val = [
	0x797979, 0x2000b2, 0x2800ba, 0x6110a2, 0x9a2079, 0xb21030, 0xa23000, 0x794100, 0x495900, 0x386900, 0x386d00, 0x306141, 0x305182, 0x000000, 0x000000, 0x000000,
	0xb2b2b2, 0x4161fb, 0x4141ff, 0x9241f3, 0xdb41c3, 0xdb4161, 0xe35100, 0xc37100, 0x8a8a00, 0x51a200, 0x49aa10, 0x49a269, 0x4192c3, 0x000000, 0x000000, 0x000000,
	0xebebeb, 0x61a2ff, 0x5182ff, 0xa271ff, 0xf361ff, 0xff61b2, 0xff7930, 0xffa200, 0xebd320, 0x9aeb00, 0x71f341, 0x71e392, 0x61d3e3, 0x797979, 0x000000, 0x000000,
	0xffffff, 0x92d3ff, 0xa2baff, 0xc3b2ff, 0xe3b2ff, 0xffbaeb, 0xffcbba, 0xffdba2, 0xfff392, 0xcbf382, 0xa2f3a2, 0xa2ffcb, 0xa2fff3, 0xa2a2a2, 0x000000, 0x000000, 
];

const nes_pal_rgb = [ "rgb(121 121 121)", "rgb(32 0 178)", "rgb(40 0 186)", "rgb(97 16 162)", "rgb(154 32 121)", "rgb(178 16 48)", "rgb(162 48 0)", "rgb(121 65 0)", "rgb(73 89 0)", "rgb(56 105 0)", "rgb(56 109 0)", "rgb(48 97 65)", "rgb(48 81 130)", "rgb(0 0 0)", "rgb(0 0 0)", "rgb(0 0 0)", "rgb(178 178 178)", "rgb(65 97 251)", "rgb(65 65 255)", "rgb(146 65 243)", "rgb(219 65 195)", "rgb(219 65 97)", "rgb(227 81 0)", "rgb(195 113 0)", "rgb(138 138 0)", "rgb(81 162 0)", "rgb(73 170 16)", "rgb(73 162 105)", "rgb(65 146 195)", "rgb(0 0 0)", "rgb(0 0 0)", "rgb(0 0 0)", "rgb(235 235 235)", "rgb(97 162 255)", "rgb(81 130 255)", "rgb(162 113 255)", "rgb(243 97 255)", "rgb(255 97 178)", "rgb(255 121 48)", "rgb(255 162 0)", "rgb(235 211 32)", "rgb(154 235 0)", "rgb(113 243 65)", "rgb(113 227 146)", "rgb(97 211 227)", "rgb(121 121 121)", "rgb(0 0 0)", "rgb(0 0 0)", "rgb(255 255 255)", "rgb(146 211 255)", "rgb(162 186 255)", "rgb(195 178 255)", "rgb(227 178 255)", "rgb(255 186 235)", "rgb(255 203 186)", "rgb(255 219 162)", "rgb(255 243 146)", "rgb(203 243 130)", "rgb(162 243 162)", "rgb(162 255 203)", "rgb(162 255 243)", "rgb(162 162 162)", "rgb(0 0 0)", "rgb(0 0 0)" ];
