const fs = require('fs');

let tohex = (x) => x.toString(16).padStart(2, '0'); 

const ent_list = [
	'ent_nothing',
];

let id_list = '';
let spawn_lo = 'ent_spawn_lo:';
let spawn_hi = 'ent_spawn_hi:';
let update_lo = 'ent_update_lo:';
let update_hi = 'ent_update_hi:';

for (const [i, ent_name] of ent_list.entries()) {
	let ent = ent_name;
	id_list += (ent + '_id').padEnd(25, ' ') + 'eqm $' + tohex(i) + "\n";
	spawn_lo += "\n\tbyte <" + ent + '_spawn';
	spawn_hi += "\n\tbyte >" + ent + '_spawn';
	update_lo += "\n\tbyte <" + ent + '_update';
	update_hi += "\n\tbyte >" + ent + '_update';
}


const out = id_list + "\n\n" + spawn_lo + "\n" + spawn_hi + "\n\n" + update_lo + "\n" + update_hi;

fs.writeFileSync('ent_tables.asm', out);
console.log(out);
