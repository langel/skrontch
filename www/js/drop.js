/*

	drag and drop handler

*/


const process = (file, data) => {
	console.log(file);
	console.log(data);
	// check for chr file
	if (file.name.endsWith('.chr')) {
		process_chr(data);
		proj.chr[file.name] = Array.from(data);
		anim_chr_new(file.name);
		skrontch_update();
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


const drop_init = () => {
	
	const droptarg = document;
	const cont = document.getElementById("containment");
	const domp = new DOMParser();
	// drop handler
	droptarg.addEventListener("drop", (e) => {
		e.preventDefault();
		cont.classList.remove("dragover");
		output.innerHTML = '';
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
}
