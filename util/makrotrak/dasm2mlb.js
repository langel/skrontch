/*
	convert 
	DASM listing export 
	to a
	Mesen LaBel file
*/

let fs = require('fs');
let input = 'listing.txt';
if (!fs.existsSync(input)) {
	console.log('failed to find ' + input);
	process.exit(1);
}

let output = './rom.mlb';
fs.writeFileSync(output, '', { flag: 'w+' });
let text = '';

let lines = fs.readFileSync(input).toString().split("\n");
for (const [i, line] of lines.entries()) {
	// zero page labels
	if (line.match(/U[0-9A-Fa-f]{4}/) !== null
	&& line.indexOf('byte.b') > 0) {
		let addr = line.substring(9, 13);
		let label = line.substring(28);
		label = label.split(' ')[0];
		text += 'R:'+addr+':'+label+"\n";
	}
	// program labels
	if (line.match(/\t\t\t\t    /g) !== null
	&& line[22] !== ' ' 
	&& line[22] !== "\t"
	//&& line[22] !== '.'
	&& !/eqm/i.test(line)) {
		let addr = parseInt(line.substring(9, 14), 16);
		addr -= 0x8000;
		addr = addr.toString(16).padStart(5, '0');
		let label = line.substring(22);
		label = label.split(' ')[0].replace(/^\./, '');
		text += 'P:'+addr+':'+label+"\n";
	}
}

fs.appendFileSync(output, text);
