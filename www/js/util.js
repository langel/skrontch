

const blank = "&#x2800;";
const br = "<br>";

// value convertors
const char = (ord) => String.fromCharCode(ord);
const ord = (char) => char.charCodeAt(0);
const tohex = (x) => x.toString(16).padStart(2, '0'); 

// DOM
const element_new = (t) => document.createElement(t);
const frame_next = () => { return new Promise(resolve => requestAnimationFrame(resolve)); }
const tobottom = () => window.scrollTo(0, document.body.scrollHeight);

// local storage
const localget = (key) => localStorage.getItem(key);
const localset = (key, val) => localStorage.setItem(key, val);

// arrays

