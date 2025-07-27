document.addEventListener('DOMContentLoaded', (event) => {
    getPatterns()
});

var patterns = [];
var builtin = [];

function showError(error) {
    emsg = 'Error fetching OTA status:' + error;
    console.error(emsg);
    document.getElementById("status").innerHTML = emsg;
    queueStatus();
}

function queueStatus() {
    // setTimeout(updateStatus, 1000);
}

function getPatterns() {
    fetch('/patterns')
        .then(response => response.json())
        .then(data => {
            patterns = data;
            getBuiltin();
        })
        .catch(error => showError(error));
    document.getElementById("status").innerHTML = "Querying Patterns...";
}
function getBuiltin() {
    fetch('/builtin')
        .then(response => response.json())
        .then(data => {
            builtin = data;
            showPatterns();
        })
        .catch(error => showError(error));
    document.getElementById("status").innerHTML = "Querying Builtin...";
}

function buildUsage(patterns, builtin) {
    const builtinSet = new Set(builtin);
    const patternsSet = new Set(patterns);

    const builtinResult = [];
    const overriddenResult = [];
    const loadedResult = [];

    // Process builtin array
    for (const item of builtin) {
        if (patternsSet.has(item)) {
            overriddenResult.push([item, "overridden"]);
            patternsSet.delete(item); // Remove to avoid duplicate processing
        } else {
            builtinResult.push([item, "builtin"]);
        }
    }

    // Process remaining items in patterns array
    for (const item of patterns) {
        if (patternsSet.has(item)) {
            loadedResult.push([item, "loaded"]);
        }
    }

    // Sort each result array alphabetically by the entry name
    builtinResult.sort((a, b) => a[0].localeCompare(b[0]));
    overriddenResult.sort((a, b) => a[0].localeCompare(b[0]));
    loadedResult.sort((a, b) => a[0].localeCompare(b[0]));

    // Concatenate the sorted arrays in the required order
    return [...builtinResult, ...overriddenResult, ...loadedResult];
}


function showPatterns() {
    let combined = buildUsage(patterns, builtin);
    const patternsDiv = document.getElementById('patterns');
    document.getElementById("status").innerHTML = "";

    // Clear any existing content in the patterns div
    patternsDiv.innerHTML = '';

    // Create a <ul> element to hold the list items
    const ul = document.createElement('ul');

    combined.forEach(([name, type]) => {
        const li = document.createElement('li');
        const nameText = document.createTextNode(name + ' ');
        const typeEm = document.createElement('em');
        const typeText = document.createTextNode(type);

        typeEm.appendChild(typeText);
        li.appendChild(nameText);
        li.appendChild(typeEm);

        if (type === 'overridden') {
            const revertButton = document.createElement('button');
            revertButton.textContent = 'Revert';
            li.appendChild(revertButton);
        } else if (type === 'loaded') {
            const deleteButton = document.createElement('button');
            deleteButton.textContent = 'Delete';
            li.appendChild(deleteButton);
        }

        ul.appendChild(li);
    });

    patternsDiv.appendChild(ul);
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
