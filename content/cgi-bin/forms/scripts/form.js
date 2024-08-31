const myForm = document.getElementById('myForm')
const showB = document.getElementById("dispCode")
const pass = document.getElementById('passCode')

showB.addEventListener("change", () => {
	if (showB.checked)
		pass.setAttribute("type", "text")
	else
	pass.setAttribute("type", "password")
})