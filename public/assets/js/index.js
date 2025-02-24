function togglePopup(popupId) {
	const popup = document.getElementById(popupId);
	const form = popup.querySelector('form');
	if (popup.style.display === 'flex') {
		popup.classList.remove('show');
		setTimeout(() => {
			popup.style.display = 'none';
		}, 300);
	} else {
		popup.style.display = 'flex';
		setTimeout(() => {
			popup.classList.add('show');
		}, 10);
	}
	if (popup.style.display === 'none') {
		popup.querySelector('form').reset();
	}

	if (popup.style.display === 'flex') {
		form.reset();
	}
}

function closePopup(event, popupId) {
	if (event.target.id === popupId) {
		togglePopup(popupId);
	}
}

async function login(event) {
	event.preventDefault();

	const email = document.getElementById('email').value;
	const password = document.getElementById('password').value;

	if (!email || !password) {
		handleError("Bitte füllen Sie alle Felder aus.");
		return;
	}

	if(loginRoute(email, password)) {
		togglePopup('loginPopup');
		window.location.href = `/dashboard?email=${email}`;
	}
}

async function register(event) {
	event.preventDefault();

	const email = document.getElementById('emailReq').value;
	const password = document.getElementById('passwordReq').value;
	const confirmPassword = document.getElementById('confirmPasswordReq').value;

	if (!email || !password || !confirmPassword) {
		handleErrorReq("Please fill in all fields.");
		return;
	}

	if (password !== confirmPassword) {
		handleError("Passwords do not match.");
		return;
	}

	if(registerRoute(email, password)) {
		togglePopup('registerPopup');
	}
}

function handleError(message) {
	const errorMessageDiv = document.getElementById('error-message');
	errorMessageDiv.textContent = message;
	errorMessageDiv.style.display = 'block';
}

function handleErrorReq(message) {
	const errorMessageDiv = document.getElementById('error-messageReq');
	errorMessageDiv.textContent = message;
	errorMessageDiv.style.display = 'block';
}
