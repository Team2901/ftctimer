// Global variable to store the AP mode status
let apModeStatus = false;

// Function to fetch the AP mode status and set the global variable
function fetchApModeStatus() {
    fetch('/status')
        .then(response => {
            if (!response.ok) {
                throw new Error(`HTTP error! status: ${response.status}`);
            }
            return response.json();
        })
        .then(data => {
            apModeStatus = data.apmode === true;
            AdjustAPUI()
        })
        .catch(error => {
            console.error('Error fetching /status:', error);
        });
}

// Synchronous function to check the AP mode status
function isInAP_Mode() {
    return apModeStatus;
}

window.addEventListener('DOMContentLoaded', fetchApModeStatus);

function AdjustAPUI() {
    // Check AP Mode and toggle form visibility
    if (isInAP_Mode()) {
        // Show the form if in AP mode
        document.getElementById('ap-mode-form').classList.remove('hidden');

        // Disable the "Check for Update" button and add the 'disabled_button' class
        const checkUpdateButton = document.getElementById('check-update-button');
        checkUpdateButton.disabled = true;  // Disable the button
        checkUpdateButton.classList.add('disabled_button');

        // Disable the "Select Network" button and add the 'active' class
        const checkNetworkButton = document.getElementById('check-network-button');
        checkNetworkButton.disabled = true;  // Disable the button
        checkNetworkButton.classList.add('active');
    }
}
