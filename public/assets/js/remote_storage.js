async function registerRoute(email, password) {
	try {
		const response = await fetch('/createAccount', {
			method: 'POST',
			headers: { 'Content-Type': 'application/json' },
			body: JSON.stringify({ email, password })
		});

		if (response.ok) {

			console.log("Registration successful");
			return true;
		} else {
			if (response.status === 400) {
				handleErrorReq("Bad request")
			} else if (response.status === 500) {
				handleErrorReq("Internl server error. Please try again later")
			} else if (response.status === 401) {
				handleErrorReq("No account for this credentials")
			} else if (response.status === 404) {
				handleErrorReq("Route not found")
			} else if (response.status === 409) {
				handleErrorReq("Conflict : Emial already exists")
			}
			return false;
		}
	} catch (error) {
		console.log(response);
		return false;
	}
}

async function loginRoute(email, password) {
	try {
		const response = await fetch('/auth/login', {
			method: 'POST',
			headers: { 'Content-Type': 'application/json' },
			body: JSON.stringify({ email, password })
		});

		if (response.ok) {
			togglePopup('loginPopup');
			console.log("Login erfolgreich");
			return true;
		} else {
			console.log(response.status)
			if (response.status === 400) {
				handleError("Bad request")
			} else if (response.status === 500) {
				handleError("Internl server error. Please try again later")
			} else if (response.status === 501) {
				handleError("No account for this credentials")
			} else if (response.status === 404) {
				handleError("Route not found")
			} else if (response.status === 401) {
				handleError("No account for this credentials")
			}
			return false;
		}
	} catch (error) {
		console.log(response);
		return false;
	}
}

async function uploadFile(file, email) {
    try {
        const response = await fetch(`/uploadFile?email=${email}&fileName=${file.name}`, {
            method: 'POST',
            body: file
        });

        if (response.ok) {
            console.log("File upload successful");
            return true;
        } else {
			console.log(response);
            return false;
        }
	} catch (error) {
		return false;
	}
}

async function getFile(fileName, email) {
    try {
        const response = await fetch(`/getFile?email=${email}&fileName=${fileName}`, {
            method: 'GET'
        });

        if (response.ok) {
            const contentType = response.headers.get('Content-Type');

            if (contentType && contentType.startsWith('image')) {
                const imageBlob = await response.blob();
                return imageBlob;
            }
            else if (contentType && contentType.startsWith('application')) {
                const textContent = await response.text();
                return textContent;
            }
            else {
                console.log('Unsupported file type:', contentType);
                return false;
            }
        } else {
            console.log(`Error fetching file: ${response.status}`);
            return false;
        }
    } catch (error) {
        console.error('Error:', error);
        return false;
    }
}



async function getFileNames(email) {
    try {
        const response = await fetch(`/getFileNames?email=${email}`, {
            method: 'GET'
        });

        if (response.ok) {
            const fileList = await response.text();

			const filesArray = fileList ? fileList.split(";") : [];

            return filesArray;
        } else {
			console.log(response);
            return [];
        }
	} catch (error) {
		return [];
	}
}

async function deleteFileRoute(fileName, email) {
    try {
        const response = await fetch(`/deleteFile?email=${email}&fileName=${fileName}`, {
            method: 'DELETE'
        });

        if (response.ok) {
			return true;
        } else {
            console.log(`Error fetching file: ${response.status}`);

        }
    } catch (error) {
        console.error('Error:', error);

    }
}


async function checkRootPassword(password) {
    try {
        const response = await fetch(`/checkRootPassword?password=${password}`, {
            method: 'GET'
        });

        if (response.ok) {
			return true;
        } else {
            console.log(`Error fetching file: ${response.status}`);
			return false;
        }
    } catch (error) {
        console.error('Error:', error);

    }
}

async function executeSkript(fileName, email) {
    try {
        const response = await fetch(`/executeSkript/test.sh?fileName=${fileName}&email=${email}`, {
            method: 'GET'
        });

        if (response.ok) {
			console.log(await response.text());
			return true;
        } else {
            console.log(`Error fetching file: ${response.status}`);
			return false;
        }
    } catch (error) {
        console.error('Error:', error);

    }
}
