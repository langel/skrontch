const {button, div} = van.tags

const calculator = (id) => {
	let lhs = van.state(null), op = null, rhs = van.state(0)

	const calc = (lhs, op, rhs) =>
		!op || lhs === null ? rhs :
		op === "+" ? lhs + rhs :
		op === "-" ? lhs - rhs :
		op === "x" ? lhs * rhs : lhs / rhs;

	const onclick = e => {
		const str = e.target.innerText;
		if (str >= "0" && str <= "9") {
			typeof rhs.val === "string" ? rhs.val += str : rhs.val = rhs.val * 10 + Number(str)
		}
		else if (str === "AC") lhs.val = op = null, rhs.val = 0
		else if (str === "+/-" && rhs.val) rhs.val = -rhs.val
		else if (str === "%" && rhs.val) rhs.val *= 0.01
		else if (str === "+" || str === "-" || str === "x" || str === "÷") {
			if (rhs.val !== null) lhs.val = calc(lhs.val, op, Number(rhs.val)), rhs.val = null
			op = str
		} 
		else if (str === "=" && op && rhs.val !== null) {
			lhs.val = calc(lhs.val, op, Number(rhs.val)), op = null, rhs.val = null;
		}
		else if (str === ".") rhs.val = rhs.val ? rhs.val + "." : "0.";
	}

	const Button = (str, key) => div({class: "button", key: key}, button(str))

	return div({id: "calculator-" + id, class: "calculator"},
		div({class: "display"}, div(() => rhs.val ?? lhs.val)),
		div({class: "panel", onclick},
			div(
				Button("AC"), 
				Button(chr_shift_left), 
				Button(chr_shift_right), 
				Button(chr_backspace, "Backspace")),
			div(
				Button("D"),
				Button("E"),
				Button("F"),
				Button("%")), 
			div(
				Button("A"),
				Button("B"),
				Button("C"),
				Button(chr_divide, "/")),
			div(
				Button("7"), 
				Button("8"), 
				Button("9"), 
				Button(chr_multiply)),
			div(
				Button("4"), 
				Button("5"), 
				Button("6"), 
				Button(chr_dash)),
			div(
				Button("1"), 
				Button("2"), 
				Button("3"), 
				Button("+")),
			div(
				//div({class: "button wide"}, 
				Button(chr_plus_minus, "-"), 
				Button("0"), 
				Button("."), 
				Button("=")),
		),
	)
}

van.add(document.getElementsByTagName('main')[0], calculator())
