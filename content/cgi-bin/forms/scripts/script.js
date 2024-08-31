const myCGI = document.getElementById("myForm");
const methodes = document.getElementsByClassName("methodes")[0];
const post = document.getElementById("post");
const nameInp = document.getElementById('name')
const removeInp = document.getElementsByClassName('delete')
let myInps = [];

for (child of removeInp) {
	myInps.push(child.innerHTML)
}

if (post.checked)
	addUp();


methodes.addEventListener("click", (e) => {
	if (e.target.value) {
		nameInp.setAttribute('placeholder', 'Enter your name')
		for (inp in myInps) {
			removeInp[inp].classList.remove('hide');
			removeInp[inp].innerHTML = myInps[inp]
		}
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
	console.log(myCGI.method)
	myCGI.submit();
	  // myForm.reset();
});
