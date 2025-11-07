

let output;
let logbox;

const logout = (logline) => {
	// XXX need different operations based on type
	//     objects and arrays have different outputs than strings
	console.log(logline);
	let l = elem_new('div')
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

	logbox = document.getElementById("logbox");

	// velocity calculator
	arc_out = document.getElementById("arctang_output");
	let velcalc = document.getElementById("arctang_vel_calc")
	velcalc.addEventListener('input', () => {
		let table = '';
		let velocity = Math.abs(Number(velcalc.value));
		if (velocity > 255) {
			output.innerText = '*ERROR* OUT OF RANGE MAX 255';
			return;
		}
		console.log(velocity);
		for (let i = 0; i < 7; i++) {
			let distance = velocity * Math.cos(i * 15 * Math.PI / 180);
			if (distance < 0.00000001) distance = 0;
			distance = distance.toFixed(8);
			let byte_hi = Math.floor(distance).toString().padStart(3, ' ');
			let byte_lo = Math.floor((distance - byte_hi) * 256).toString().padStart(3, ' ');
			table += "\n\tbyte    " + byte_lo + ", " + byte_hi + "    ; " + distance;
		}
		arc_out.innerText = 'arctang velocity calculator output:' + table;
	});

	// velocity calculator
	let dist_in = document.getElementById("distance_calc")
	let ang_in = document.getElementById("angle_calc")
	let distangcalc = () => {
		distance = Math.abs(Number(dist_in.value));
		angle = Math.abs(Number(ang_in.value) * (Math.PI / 180));
		console.log(distance);
		console.log(angle);
		let x = distance * Math.cos(angle);
		let y = distance * Math.sin(angle);
		dist_out = document.getElementById("distance_output");
		dist_out.innerText = "x: " + x + " , y: " + y;
		dist_out.innerText += "\n";
		dist_out.innerText += Math.floor(x) + ":" + tohex(Math.floor((x % 1) * 256)) + " ";
		dist_out.innerText += Math.floor(y) + ":" + tohex(Math.floor((y % 1) * 256));
	}
	dist_in.addEventListener('input', distangcalc);
	ang_in.addEventListener('input', distangcalc);


	console.log('core intiialized');
});
