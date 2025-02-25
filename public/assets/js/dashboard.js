function getEmailFromURL() {
    const urlParams = new URLSearchParams(window.location.search);
	return "jkarras@42wolfsburg.com";
    return urlParams.get('email');
}

async function initDashboard() {
    const email = getEmailFromURL();
    if (email) {
        document.getElementById('username').innerText = email;
        await renderFiles(email);
    } else {
        console.error('No email found in the URL.');
    }
}

async function renderFiles(email) {
    const files = [
        { name: 'File1.txt', size: '1MB' },
        { name: 'File2.png', size: '2MB' },
        { name: 'File3.pdf', size: '500KB' }
    ];

    const fileContainer = document.getElementById('file-container');
    fileContainer.innerHTML = '';

    files.forEach(file => {
        const fileDiv = document.createElement('div');
        fileDiv.classList.add('file-item');
        fileDiv.innerHTML = `<p>${file.name} - ${file.size}</p>`;
        fileContainer.appendChild(fileDiv);
    });
}

function openUploadDialog() {
    document.getElementById('file-input').click();
}

function handleFileUpload(event) {
    const file = event.target.files[0];

    if (!file) {
        return;
    }

    console.log('Selected file MIME type:', file.type);

    const allowedTypes = ['image/jpeg', 'image/png', 'application/x-sh', 'application/x-shellscript'];
    if (!allowedTypes.includes(file.type)) {
        alert("Invalid file type. Please upload JPG, PNG, or SH files.");
        return;
    }

    alert(`File uploaded: ${file.name}`);

	uploadFile(file, getEmailFromURL());
}
