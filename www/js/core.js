

let output;
let logbox;

const logout = (logline) => {
	// XXX need different operations based on type
	//     objects and arrays have different outputs than strings
	console.log(logline);
	let l = element_new('div')
	l.innerHTML = logline;
	logbox.appendChild(l);
}

const logclear = () => logbox.innerHTML = '';

const process = (file, data) => {
	output.innerHTML += file.name + "\n";
	logout(blank);
	logout('loading ' + file.name + ' . . . ');
	console.log(file);
	console.log(data);
	// check for chr file
	if (file.name.endsWith('.chr')) {
		process_chr(data);
	}
	// check for nes file
	// check for nsf file
	if (data[0] == 78 && // N
	    data[1] == 69 && // E
		 data[2] == 83 && // S
		 data[3] == 77 && // M
		 data[4] == 26) { // 0x1a
		process_nsf(data);
	}
	// check for pce file
	// ask user for filetype
}

window.addEventListener("DOMContentLoaded", () => {
	const droptarg = document;
	const cont = document.getElementById("containment");
	const domp = new DOMParser();
	output = document.getElementById("output");
	logbox = document.getElementById("logbox");
	// drop handler
	droptarg.addEventListener("drop", (e) => {
		e.preventDefault();
		cont.classList.remove("dragover");
		output.innerHTML = '';
		logclear();
		[...e.dataTransfer.items].forEach((item, i) => {
			if (item.kind === 'file') {
				const file = item.getAsFile();
				const r = new FileReader();
				r.readAsArrayBuffer(file);
				//r.readAsBinaryString(file);
				//r.readAsText(file);
				r.onload = () => {
					process(file, new Uint8Array(r.result));
				};
			}
		});
	});
	// drag hover
	droptarg.addEventListener("dragover", (e) => {
		e.preventDefault();
		cont.classList.add("dragover");
	});
	// drag end
	droptarg.addEventListener("dragleave", (e) => {
		e.preventDefault();
		cont.classList.remove("dragover");
	});
	// velocity calculator
	let velcalc = document.getElementById("velcalc")
	velcalc.addEventListener('input', () => {
		let table = '';
		let velocity = Math.abs(Number(velcalc.value));
		console.log(velocity);
		for (let i = 0; i < 7; i++) {
			let distance = velocity * Math.cos(i * 15 * Math.PI / 180);
			if (distance < 0.0000000001) distance = 0;
			let byte_hi = Math.floor(distance).toString().padStart(3, ' ');
			let byte_lo = Math.floor((distance - byte_hi) * 256).toString().padStart(3, ' ');
			table += "\n\tbyte    " + byte_lo + ", " + byte_hi + "    ; " + distance;
		}
		output.innerText = 'velcalc' + table;
	});
	console.log('core intiialized');
});
