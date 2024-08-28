const myCGI = document.getElementById("myForm");
const methodes = document.getElementsByClassName("methodes")[0];
const post = document.getElementById("post");
const upl = document.getElementById('up')

const addUp = () => {
	upl.classList.remove("hide");
	upl.innerHTML = `
		<label for="upload">Upload file</label>
		<input required name="upload" id="upload" type="file" />`;
}

if (post.checked)
	addUp();

methodes.addEventListener("click", (e) => {
	if (e.target.value == "POST") {
		addUp();
	}
	else if (e.target.value) {
		upl.classList.add("hide");
		upl.innerHTML = ``;
	}
});

myCGI.addEventListener("submit", (e) => {
	e.preventDefault();
	const myEntries = {};
	// const [name, email, msg, language, methode] = [
	// 	document.getElementById('name').value,
	// 	document.getElementById('email').value,
	// 	document.getElementById('message').value,
	// 	document.querySelector('input[name="lang"]:checked').value,
	// 	document.querySelector('input[name="methode"]:checked').value
	// ];
	const data = new FormData(myCGI);
	data.forEach((value, key) => {
		myEntries[key] = value;
	});
	myCGI.action = "/cgi-bin/cgi." + myEntries.language;
	myCGI.method = myEntries.methode;
	myCGI.submit();
	// document.getElementById('name').value = '';
	// document.getElementById('email').value = '';
	// document.getElementById('message').value = '';
});
