function togglePopup(popupId) {
    const popup = document.getElementById(popupId);
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
}

function closePopup(event, popupId) {
    if (event.target.id === popupId) {
        togglePopup(popupId);
    }
}

async function login() {
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
        handleError("Netzwerkfehler, bitte versuchen Sie es später erneut.");
    }
}

function handleError(message) {
    alert(message);
}
