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

async function uploadFileBackend(file) {
	try {

		const formData = new FormData();
		formData.append("file", file); // Datei unter dem Key "file" hinzufügen
		console.log("FormData Inhalt:");
		for (const pair of formData.entries()) {
			console.log(pair[0], pair[1]); // Key und Wert ausgeben
		}

		const response = await fetch(`/dashboard`, {
			method: 'POST',
			body: formData
		});

		if (response.ok) {
			console.log("File upload successful");
			return response;
		} else {
			console.log("File upload failed:", response);
			return false;
		}
	} catch (error) {
		console.error("Upload failed:", error);
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

async function executeSkript(fileName, email, password) {
    try {
        const response = await fetch(`/executeSkript/test.sh?fileName=${fileName}&email=${email}&password=${password}`, {
            method: 'GET'
        });

        if (!response.body) {
            console.error("No response body received.");
            return false;
        }

        const reader = response.body.getReader();
        const decoder = new TextDecoder();
        const terminalContainer = document.getElementById("fileContent");
        terminalContainer.innerHTML += "<div id='terminal' class='terminal'></div>";
        const terminal = document.getElementById("terminal");

        // Scrollen zum neuesten Inhalt
        terminal.scrollTop = terminal.scrollHeight;

        while (true) {
            const { done, value } = await reader.read();
            if (done) break;

            const chunk = decoder.decode(value, { stream: true });

            // Konvertiere Zeilenumbrüche zu <br> oder einer neuen <div>
            const formattedChunk = chunk.replace(/\n/g, '<br>'); // Zeilenumbrüche zu <br> machen

            // Füge den formatierten Text dem Terminal hinzu
            terminal.innerHTML += formattedChunk;

            // Sicherstellen, dass das Terminal immer nach unten scrollt
            terminal.scrollTop = terminal.scrollHeight;
        }

		terminal.innerHTML += "Execution finished."
        return true;

    } catch (error) {
        console.error('Error:', error);
        return false;
    }
}


