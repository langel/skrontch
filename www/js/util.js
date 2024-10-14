

const blank = "&#x2800;";
const br = "<br>";

let element_new = (t) => document.createElement(t);
let frame_next = () => { return new Promise(resolve => requestAnimationFrame(resolve)); }
let localget = (key) => localStorage.getItem(key);
let localset = (key, val) => localStorage.setItem(key, val);
let tobottom = () => window.scrollTo(0, document.body.scrollHeight);
let tohex = (x) => x.toString(16).padStart(2, '0'); 
