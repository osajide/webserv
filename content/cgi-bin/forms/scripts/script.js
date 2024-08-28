const myCGI = document.getElementById("myForm");
const methodes = document.getElementsByClassName("methodes")[0];

methodes.addEventListener("click", (e) => {
	const upl = document.getElementById('up')
  if (e.target.value == "POST") {
    upl.classList.remove("hide");
    upl.innerHTML = `
			<label for="upload">Upload file</label>
			<input required name="upload" id="upload" type="file" placeholder="click here to upload" />`;
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
