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

	try {
		const response = await fetch('/auth/login', {
			method: 'POST',
			headers: { 'Content-Type': 'application/json' },
			body: JSON.stringify({ email, password })
		});

		if (response.ok) {
			togglePopup('loginPopup');
			console.log("Login erfolgreich");
		} else {
			const errorData = await response.json();
			handleError(errorData.message || "Login fehlgeschlagen");
		}
	} catch (error) {
		handleError("Network error, please try again later");
	}
}

async function register(event) {
	event.preventDefault();

	const email = document.getElementById('email').value;
	const password = document.getElementById('password').value;
	const confirmPassword = document.getElementById('confirmPassword').value;

	if (!email || !password || !confirmPassword) {
		handleError("Please fill in all fields.");
		return;
	}

	if (password !== confirmPassword) {
		handleError("Passwords do not match.");
		return;
	}

	try {
		const response = await fetch('/createAccount', {
			method: 'POST',
			headers: { 'Content-Type': 'application/json' },
			body: JSON.stringify({ email, password })
		});

		if (response.ok) {
			togglePopup('registerPopup');
			console.log("Registration successful");
		} else {
			const errorData = await response.json();
			handleError(errorData.message || "Registration failed");
		}
	} catch (error) {
		handleError("Network error, please try again later.");
	}
}

function handleError(message) {
	const errorMessageDiv = document.getElementById('error-message');
	errorMessageDiv.textContent = message;
	errorMessageDiv.style.display = 'block';
}
