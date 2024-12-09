
const clipboard_set = (key, val) => {
	if (!proj.hasOwnProperty('clipboard')) proj.clipboard = {};
	proj.clipboard[key] = val;
	skrontch_update();
}

const clipboard_get = (key) => {
	if (!proj.clipboard.hasOwnProperty(key)) {
		// XXX could use a gui feedback
		console.log('undefined clipboard key');
	}
	else return proj.clipboard[key];
}
