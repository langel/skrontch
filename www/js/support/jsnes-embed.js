
//let nes = new jsnes.NES({

let emulator;
let jsnes_d = {};

const new_nes = (canvas_id, rom_path) => {
	// video
	jsnes_d.width = 256;
	jsnes_d.height = 240;
	jsnes_d.v_size = jsnes_d.width * jsnes_d.height;
	jsnes_d.v_canvas = document.getElementById(canvas_id);
	jsnes_d.v_context = jsnes_d.v_canvas.getContext('2d');
	jsnes_d.v_context.fillStyle = 'black';
	jsnes_d.v_context.fillRect(0, 0, jsnes_d.width, jsnes_d.height);
	jsnes_d.image = jsnes_d.v_context.getImageData(0, 0, jsnes_d.width, jsnes_d.height);
	let frame_buffer = new ArrayBuffer(jsnes_d.image.data.length);
	jsnes_d.frame_u8 = new Uint8ClampedArray(frame_buffer);
	jsnes_d.frame_u32 = new Uint32Array(frame_buffer);
	jsnes_d.v_callback = () => {
		window.requestAnimationFrame(jsnes_d.v_callback);
		jsnes_d.image.data.set(jsnes_d.frame_u8);
		jsnes_d.v_context.putImageData(jsnes_d.image, 0, 0);
		emulator.frame();
		//console.log('ass');
	};
	// audio
	jsnes_d.a_buffer_size = 512;
	jsnes_d.sample_count = 1024 << 2;
	jsnes_d.sample_mask = jsnes_d.sample_count - 1;
	jsnes_d.sample_left = new Float32Array(jsnes_d.sample_count);
	jsnes_d.sample_right = new Float32Array(jsnes_d.sample_count);
	jsnes_d.a_wc = 0; // write counter
	jsnes_d.a_rc = 0; // read counter
	jsnes_d.a_callback = (e) => {
		let dst = e.outputBuffer;
		let len = dst.length;
		if (((jsnes_d.a_wc - jsnes_d.a_rc) & jsnes_d.sample_mask) < jsnes_d.a_buffer_size) emulator.frame();
		let dst_left = dst.getChannelData(0);
		let dst_right = dst.getChannelData(1);
		for (let i = 0; i < len; i++) {
			jsnes_d.a_rc = (jsnes_d.a_rc + 1) & jsnes_d.sample_mask;
			dst_left = jsnes_d.sample_left[jsnes_d.a_rc];
			dst_right = jsnes_d.sample_right[jsnes_d.a_rc];
		}
	};
	jsnes_d.a_context = new window.AudioContext();
	jsnes_d.a_proc = jsnes_d.a_context.createScriptProcessor(jsnes_d.a_buffer_size, 0, 2);
	jsnes_d.a_proc.onaudioprocess = jsnes_d.a_callback;
	jsnes_d.a_proc.connect(jsnes_d.a_context.destination);
	// emulator
	emulator = new jsnes.NES({
		onFrame: (buffer_24) => {
			for (let i = 0; i < jsnes_d.v_size; i++) {
				jsnes_d.frame_u32[i] = 0xff000000 | buffer_24[i];
			}
		},
		onAudioSample: (left, right) => {
			jsnes_d.sample_left[jsnes_d.a_wc] = left;
			jsnes_d.sample_right[jsnes_d.a_wc] = right;
			jsnes_d.a_wc = (jsnes_d.a_wc + 1) & jsnes_d.sample_mask;
		},
	});
	// ROM load and boot
	let nes_rom = '';
	let req = new XMLHttpRequest();
	req.open('GET', rom_path);
	req.overrideMimeType('text/plain; charset=x-user-defined');
	req.onerror = () => console.log(req.statusText + ' :: Error loading ' + rom_path);
	req.onload = () => {
		if (req.status === 200) {
			console.log('booting :: ' + rom_path);
			// loader
			emulator.loadROM(req.responseText);
			// bootup
			window.requestAnimationFrame(jsnes_d.v_callback);
		}
		else req.onerror();
	};
	req.send();
};

window.onload = () => {
	new_nes('jsnes-canvas', 'util/makrotrak/makrotrak.nes');
};
