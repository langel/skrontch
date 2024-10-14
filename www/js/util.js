

const blank = "&#x2800;";
const br = "<br>";

// value convertors
let char = (ord) => String.fromCharCode(ord);
let ord = (char) => char.charCodeAt(0);
let tohex = (x) => x.toString(16).padStart(2, '0'); 

// DOM
let element_new = (t) => document.createElement(t);
let frame_next = () => { return new Promise(resolve => requestAnimationFrame(resolve)); }
let tobottom = () => window.scrollTo(0, document.body.scrollHeight);

// local storage
let localget = (key) => localStorage.getItem(key);
let localset = (key, val) => localStorage.setItem(key, val);
