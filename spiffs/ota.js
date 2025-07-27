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
        })
        .catch(error => {
            console.error('Error fetching /status:', error);
        });
}
// Synchronous function to check the AP mode status
function isApMode() {
    return apModeStatus;
}

window.addEventListener('load', fetchApModeStatus);

document.addEventListener('DOMContentLoaded', (event) => {
    otaQuery();
});

function showError(error) {
    let emsg = 'Error fetching OTA status:' + error;
    console.error(emsg);
    document.getElementById("status").innerHTML = emsg;
    queueStatus();
}

function queueStatus() {
    setTimeout(updateStatus, 1000);
}

function otaQuery() {
    fetch('/otaquery')
        .then(response => { queueStatus(); })
        .catch(error => showError(error));
    document.getElementById("status").innerHTML = "Updating...";
}
function updateStatus() {
    fetch('/otastatus')
        .then(response => response.json())
        .then(data => {
            document.getElementById("currentVersion").innerText = data.current;
            document.getElementById("newVersion").innerText = data.new;
            document.getElementById("otadate").innerText = data.date;
            document.getElementById("otatime").innerText = data.time;
            document.getElementById("otasize").innerText = data.size;

            let uptext = ""
            // If we have the same version then we need to change the text of the upgrade button
            if (data.current === data.new) {
                uptext = "Reinstall " + data.new;
            } else {
                uptext = "Download " + data.new;
            }
            document.getElementById("upbtn").innerText = uptext;
            updateProgress(data.size, data.read);
            queueStatus();
        })
        .catch(error => showError(error));
}

function showProgress() {
    var element = document.getElementById("progress");
    element.classList.remove("hidden");
}

function updateProgress(size, read) {
    var percentComplete = (read / size) * 100;
    if (!isFinite(percentComplete)) {
        percentComplete = 0;
    }
    document.getElementById("progressBar").value = percentComplete;
    document.getElementById("progressText").innerText = `Update Progress: ${percentComplete.toFixed(2)}%`;
}

function upgradeFirmware() {
    var xhttp = new XMLHttpRequest();
    xhttp.onreadystatechange = function () {
        if (xhttp.readyState == 4) {
            if (xhttp.status == 200) {
                document.getElementById("status").innerHTML = "OTA Update Requested";
            } else {
                document.getElementById("status").innerHTML = "OTA Update Failed: " + xhttp.responseText;
            }
        }
    };
    xhttp.open("POST", "/ota", true);
    xhttp.send();
    document.getElementById("status").innerHTML = "Updating...";
    document.getElementById("progressContainer").style.display = "block";
    document.getElementById("progressBar").value = 0;
    document.getElementById("progressText").innerText = "Update Progress: 0%";
    showProgress();
}
