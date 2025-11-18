let fs = require('fs');
let http = require('http');
let url = require('url');

const root = '../';
let server;


const file_serve = (file, res) => {
	if (!fs.existsSync(file)) {
		res.writeHead(404);
		res.write(file + " :: ");
		res.write('file unfound :(');
		res.end();
		console.log('missing file: ' + file);
	}
	else {
		let mime = {'Content-Type': 'application/octet-stream;'};
		// image types
		if (file.includes('.bmp')) mime = {'Content-Type': 'image/bmp;'};
		if (file.includes('.gif')) mime = {'Content-Type': 'image/gif;'};
		if (file.includes('.jpg')) mime = {'Content-Type': 'image/jpeg;'};
		if (file.includes('.jpeg')) mime = {'Content-Type': 'image/jpeg;'};
		if (file.includes('.png')) mime = {'Content-Type': 'image/png;'};
		// text types
		if (file.includes('.css')) mime = {'Content-Type': 'text/css; charset=utf-8;'};
		if (file.includes('.htm')) mime = {'Content-Type': 'text/html; charset=utf-8;'};
		if (file.includes('.html')) mime = {'Content-Type': 'text/html; charset=utf-8;'};
		if (file.includes('.js')) mime = {'Content-Type': '; charet=utf-8;'};
		if (file.includes('.json')) mime = {'Content-Type': 'text/javascript; charset=utf-8;'};
		if (file.includes('.txt')) mime = {'Content-Type': 'text/plain;'};
		// binary types
		if (file.includes('.zip')) mime = {'Content-Type': 'application/zip;'};
		res.writeHead(200, mime);
		res.write(fs.readFileSync(file));
		res.end();
	}
}


const route = (path, res) => {
	let file = '';
	if (path[0] == '') {
		file = root + 'www/index.html';
	}
	else if (path[0] == 'util') {
		file = root + 'util/' + path.slice(1).join('/');
	}
	else file = root + 'www/' + path.join('/');
	console.log('route: ' + file);
	file_serve(file, res);
};


exports.start = (port_id) => {
	server = http.createServer(function(req, res) {
		console.log('request: ' + req.url);
		const request = url.parse(req.url, true);
		if (!request.pathname) res.writeHead(500).end();
		let uri;
		try {
			uri = decodeURI(request.pathname);
		} catch (e) {
			uri = '';
		}
		let path = uri.split("/");
		path.shift();
		route(path, res);
	}).listen(port_id, '0.0.0.0');
};
