

async function initDashboard() {

}

function openUploadDialog() {
    document.getElementById('file-input').click();
}

async function uploadFileTmp(event) {
	const file = event.target.files[0];

	if (!file) {
		return ;
	}

	console.log('Selected file MIME type:', file.type);

	const allowedTypes = ['image/jpeg', 'image/png', 'application/x-sh', 'application/x-shellscript'];
    if (!allowedTypes.includes(file.type)) {
        alert("Invalid file type. Please upload JPG, PNG, or SH files.");
        return;
    }
	const tmp = await uploadFileBackend(file);
	console.log(tmp);
}





































async function renderFiles(email) {
    const fileNames = await getFileNames();
    const fileContainer = document.getElementById('file-container');
    fileContainer.innerHTML = '';

    for (let i = 0; i < fileNames.length; i++) {
        const fileName = fileNames[i];
        const file = await getFile(fileName, email);

        const card = document.createElement('div');
        card.classList.add('card');

        const trashIcon = document.createElement('span');
        trashIcon.classList.add('trash-icon');
        trashIcon.innerHTML = '&times;';
        trashIcon.onclick = (event) => {
            event.stopPropagation();
            deleteFile(fileName, email);
        };

        const cardBody = document.createElement('div');
        cardBody.classList.add('card-body');

        const title = document.createElement('h5');
        title.classList.add('card-title');
        title.textContent = fileName;

        const preview = document.createElement('p');
        preview.classList.add('card-text');

        if (fileName.endsWith('.sh')) {
            const fileContent = file.slice(0, 100);
            preview.textContent = fileContent;
        } else if (file instanceof Blob && file.type.startsWith('image')) {
            const image = document.createElement('img');
            image.src = URL.createObjectURL(file);
            preview.appendChild(image);
        }

        cardBody.appendChild(title);
        cardBody.appendChild(preview);
        card.appendChild(trashIcon);
        card.appendChild(cardBody);

        fileContainer.appendChild(card);

        card.onclick = () => openFileModal(fileName, file, email);
    }
}

function openFileModal(fileName, file, email) {
    const modal = $('#fileModal');
    const fileContentDiv = $('#fileContent');
    fileContentDiv.empty();
    const executeBtn = $('#executeBtn');

    if (fileName.endsWith('.sh')) {
        const scriptContent = document.createElement('pre');
        scriptContent.textContent = file;
        fileContentDiv.append(scriptContent);

        executeBtn.show();
    } else if (file instanceof Blob && file.type.startsWith('image')) {
        const img = document.createElement('img');
        img.src = URL.createObjectURL(file);
        img.style.width = '100%';
        fileContentDiv.append(img);

        executeBtn.hide();
    }
	document.getElementById("executeBtn").onclick = function() {
		executeFile(fileName, email);
	};

    modal.modal('show');
}

function closeModal() {
    const modal = $('#fileModal');
    modal.modal('hide');

    $('.modal-backdrop').remove();

    $('body').removeClass('modal-open');
}


async function deleteFile(fileName, email) {
	await deleteFileRoute(fileName, email);
	await renderFiles(email);
}

async function executeFile(fileName, email) {
	document.getElementById("fileContent").innerHTML += `
	<div id="rootpasswordContainer">
		<label for="rootPassword">Please enter root password:</label>
        <input type="password" id="rootPassword" placeholder="Root Password" required>
        <button type="button" id="submitPasswordBtn">Submit</button>
        <p id="errorMessage" style="color: red; display: none;">Incorrect password. Please try again.</p>
	</div>
	`
	document.getElementById("executeBtn").style.display = "none";
	document.getElementById("submitPasswordBtn").onclick = async function() {
        const password = document.getElementById("rootPassword").value;

		if (await checkRootPassword(password)) {
			executeSkript(fileName, email, password);
			document.getElementById("rootpasswordContainer").style.display = "none";

		} else {
			document.getElementById("errorMessage").style.display = 'block';
		}
    };
}




