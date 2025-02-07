const {button, div} = van.tags

const calculator = (id) => {
	let lhs = van.state(null), op = null, rhs = van.state(0)

	const calc = (lhs, op, rhs) =>
		!op || lhs === null ? rhs :
		op === "+" ? lhs + rhs :
		op === chr_dash ? lhs - rhs :
		op === chr_multiply ? lhs * rhs : 
		op === chr_divide ? lhs / rhs :
		op === "&" ? lhs & rhs :
		op === "|" ? lhs | rhs :
		op === "^" ? lhs ^ rhs :
		op === "%" ? lhs % rhs : 0;

	const onclick = e => {
		const str = e.target.innerText;
		console.log('wewew', str, e);
		if (str >= "0" && str <= "9") {
			typeof rhs.val === "string" ? rhs.val += str : rhs.val = rhs.val * 10 + Number(str)
		}
		else if (str === "AC") lhs.val = op = null, rhs.val = 0
		else if (str === "+/-" && rhs.val) rhs.val = -rhs.val
		else if (str === "~" && rhs.val) ~ rhs.val
		else if (str == chr_shift_left) rhs.val <<= 1;
		else if (str == chr_shift_right) rhs.val >>>= 1;
		else if (["+", chr_dash, chr_multiply, chr_divide, "&", "|", "^", "%"].includes(str)) {
			if (rhs.val !== null) lhs.val = calc(lhs.val, op, Number(rhs.val)), rhs.val = null
			op = str
		} 
		else if (str === "=" && op && rhs.val !== null) {
			lhs.val = calc(lhs.val, op, Number(rhs.val)), op = null, rhs.val = null;
		}
		else if (str === ".") rhs.val = rhs.val ? rhs.val + "." : "0.";
	}

	const Button = (str, key) => div({class: "button", key: key}, button(str))

	const keys = [
		// row 1
		[ 'AC', 'escape', 'all_clear' ],
		[ chr_shift_left, '<', 'shift_left' ],
		[ chr_shift_right, '>', 'shift_right' ],
		[ chr_backspace, 'backspace', 'backspace' ],
		// row 2
		[ '&', '&', 'and' ],
		[ '|', '|', 'or' ],
		[ '^', '^', 'xor' ],
		[ '~', '~', 'not' ],
		// row 3
		[ 'D', 'd', 'd' ],
		[ 'E', 'e', 'e' ],
		[ 'F', 'f', 'f' ],
		[ '%', '%', 'modulo' ],
		// row 4
		[ 'A', 'a', 'a' ],
		[ 'B', 'b', 'b' ],
		[ 'C', 'c', 'c' ],
		[ chr_divide, '/', 'divide' ],
		// row 5
		[ '7', '7', '7' ],
		[ '8', '8', '8' ],
		[ '9', '9', '9' ],
		[ chr_multiply, '*', 'multiply' ],
		// row 6
		[ '4', '4', '4' ],
		[ '5', '5', '5' ],
		[ '6', '6', '6' ],
		[ chr_dash, '-', 'minus' ],
		// row 7
		[ '1', '1', '1' ],
		[ '2', '2', '2' ],
		[ '3', '3', '3' ],
		[ '+', '+', 'plus' ],
		// row 8
		[ chr_plus_minus, 'i', 'invert' ],
		[ '0', '0', '0' ],
		[ '.', '.', 'dot' ],
		[ '=', '=', 'equals' ],
	];

	let key_buttons = [];
	for (let i = 0; i < keys.length; i++) {
		key_buttons.push(Button(keys[i][0], keys[i][1]));
	}
	let key_rows = [];
	for (let i = 0; i < key_buttons.length / 4; i++) {
		key_rows.push(div(
			key_buttons[i * 4 + 0],
			key_buttons[i * 4 + 1],
			key_buttons[i * 4 + 2],
			key_buttons[i * 4 + 3],
		));
	}

	return div({id: "calculator-" + id, class: "calculator"},
		div({class: "display mono"}, div(() => rhs.val ?? lhs.val)),
		div({class: "keypad", onclick}, ...key_rows),
	)
}

van.add(document.getElementById('calc'), calculator())
