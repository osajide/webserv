const myCGI = document.getElementById("myForm");
const methodes = document.getElementsByClassName("methodes")[0];
const post = document.getElementById("post");
const deleteBut = document.getElementById("delete");
const upl = document.getElementById('up')
const nameInp = document.getElementById('name')
const removeInp = document.getElementsByClassName('delete')
let myInps = [];

for (child of removeInp) {
	myInps.push(child.innerHTML)
}


const addUp = () => {
	upl.classList.remove("hide");
	upl.innerHTML = `
		<label for="upload">Upload file</label>
		<input name="upload" id="upload" type="file" />`;
}

const remInput = () => {
	nameInp.setAttribute('placeholder', 'Enter file name to delete')
	for (inp of removeInp) {
		inp.innerHTML = ``
		inp.classList.add('hide');
	}
}

if (post.checked)
	addUp();

if (deleteBut.checked)
	remInput()

methodes.addEventListener("click", (e) => {
	if (e.target.value == "POST") {
		addUp();
	}
	else if (e.target.value) {
		upl.classList.add("hide");
		upl.innerHTML = ``;
	}

	if (e.target.value == "DELETE") {
		remInput()
	}
	else if (e.target.value) {
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
		if (key != 'upload' || value.name.length){
			if (key == 'upload')
				myCGI.enctype= "multipart/form-data"
			myEntries[key] = value;
		}
		else
			myCGI.enctype= ""
	});
	myCGI.action = "/cgi-bin/cgi." + myEntries.language;
	myCGI.method = myEntries.methode;
	myCGI.submit();
	  // myForm.reset();
});
