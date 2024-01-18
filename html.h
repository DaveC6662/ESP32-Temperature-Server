/*
Header: html.h
Author: Davin Chiupka
Date: December 3, 2023
Last Updated: January 18th, 2024

Description:
This header file provides HTML templates and JavaScript functions for the ESP32-based web server application.

HTML Templates:
- index_html: Login page HTML with a form for configuring WiFi settings.
- data_html: Temperature server page HTML to display real-time temperature data.

Usage:
- Include this header file in your main Arduino sketch to access HTML templates and JavaScript functions.
- The HTML templates are designed for use with the ESPAsyncWebServer library.

Note: Ensure that the ESPAsyncWebServer library is properly installed and configured in your Arduino IDE.
*/

#ifndef HTML_H
#define HTML_H

/**
 * HTML content for the login page, stored in Flash memory.
 *
 * This constant contains the HTML markup for the device's login page, which is used
 * when setting up the WiFi connection for the ESP32. The page includes a form for
 * entering WiFi credentials and selecting the security type (WPA2-Personal or WPA2-Enterprise).
 *
 * The HTML uses inline CSS for styling and a simple JavaScript function to handle changes
 * in the security type selection. This function adjusts the visibility of additional
 * input fields based on the selected security type.
 *
 * The form data is sent to the "/get" endpoint when the user submits the form.
 *
 * Note: The 'PROGMEM' keyword is used to store the HTML in the program's flash memory
 * instead of RAM, optimizing memory usage on the ESP32.
 */
const char index_html[] PROGMEM = R"rawliteral(
<!DOCTYPE HTML>
<html>

<head>
    <title>Login Page</title>
    <meta name="viewport" content="width=device-width, initial-scale=1">
    <style>
        body {
            font-family: Arial, sans-serif;
            background-color: #f4f4f4;
            text-align: center;
            margin: 20px;
        }

        h3 {
            color: #333;
        }

        form {
            width: 300px;
            margin: 0 auto;
            background-color: #fff;
            padding: 20px;
            border-radius: 8px;
            box-shadow: 0 0 10px rgba(0, 0, 0, 0.1);
        }

        .security-fields {
            display: none;
        }

        input,
        select {
            width: 100%;
            padding: 10px;
            margin: 8px 0;
            box-sizing: border-box;
            border: 1px solid #ccc;
            border-radius: 4px;
        }

        input[type="submit"] {
            background-color: #4caf50;
            color: white;
            cursor: pointer;
        }

        input[type="submit"]:hover {
            background-color: #45a049;
        }

        .disabled {
            background-color: #ddd;
            color: #888;
            cursor: not-allowed;
        }

    </style>
    <script>
        function handleSecurityChange() {
            var securitySelect = document.getElementById("security");
            var securityFields = document.getElementById("security-fields");
            var usernameField = document.getElementById("username-field");
            var passwordInput = document.querySelector('input[name="Expiry Date"]');
            var submitButton = document.getElementById("submit-button");

            if (securitySelect.value === "WPA2-Enterprise") {
                securityFields.style.display = "block";
                usernameField.style.display = "block";
            } else if (securitySelect.value === "WPA2-Personal") {
                securityFields.style.display = "block";
                usernameField.style.display = "none";
            } else {
                securityFields.style.display = "none";
                usernameField.style.display = "none";
            }
        }

    </script>
</head>

<body>
    <h3>Esp32 Wifi Connection Information</h3>
    <br><br>
    <form action="/get">
        <div id="security-fields" class="security-fields">
            <br>
            SSID: <input type="text" name="SSID">
            <br>
            <div id="username-field">
                Username: <input type="text" name="Username">
                <br>
            </div>
            Password: <input type="password" name="Password">
            <br>
            Access Code: <input type="password" name="Passcode">
            <br>
        </div>
        Security:
        <select name="Security" id="security" onchange="handleSecurityChange()">
            <option value=""></option>
            <option value="WPA2-Personal">WPA2 Personal</option>
            <option value="WPA2-Enterprise">WPA2 Enterprise</option>
        </select>
        <br>
        <input type="submit" id="submit-button" value="Submit">
    </form>
</body>

</html>
)rawliteral";

/**
 * HTML content for the data page, stored in Flash memory.
 *
 * This constant contains the HTML markup for the device's data page, which is used
 * to display temperature data and settings on the ESP32. The page includes a table
 * for showing temperature readings and a settings panel for configuring temperature thresholds
 * and timer delay.
 *
 * The HTML uses inline CSS for styling and JavaScript for dynamic content loading and interaction.
 * The JavaScript fetches temperature data and device information, updates the table,
 * handles settings changes, and formats uptime display.
 *
 * The settings panel allows the user to adjust minimum and maximum temperature thresholds,
 * as well as the timer delay for temperature readings. These settings are sent to the "/updateSettings"
 * endpoint when the user saves them.
 *
 * Note: The 'PROGMEM' keyword is used to store the HTML in the program's flash memory
 * instead of RAM, optimizing memory usage on the ESP32.
 */
const char data_html[] PROGMEM = R"rawliteral(
<!DOCTYPE HTML>
<html>
<head>
    <title>Data Page</title>
    <meta charset="UTF-8">
    <meta name="viewport" content="width=device-width, initial-scale=1">
    <style>
        html {
            font-family: 'Arial', sans-serif;
        }

        body {
            margin: 0;
            padding: 0;
            background-color: #f5f5f5;
        }

        h2 {
            text-align: center;
            margin: 0;
            padding: 20px;
            background-color: black;
            color: #fff;
            letter-spacing: 2px;
            text-transform: uppercase;
        }

        table {
            width: 100%;
            border-collapse: collapse;
            box-shadow: 0 4px 8px rgba(0, 0, 0, 0.1);
            background-color: #fff;
        }

        th,
        td {
            padding: 15px;
            text-align: center;
            border-bottom: 1px solid #ddd;
        }

        th {
            background-color: #333;
            color: #fff;
            cursor: pointer;
        }

        .menu-container {
            display: flex;
            justify-content: space-between;
            align-items: center;
            background-color: black;
            padding-left: 10px;
        }

        .menu-icon {
            color: white;
            font-size: 1em;
            cursor: pointer;
            margin: 0;
            padding: 20px;
            color: #fff;
            letter-spacing: 2px;
            text-transform: uppercase;
        }

        .menu-icon:hover {
            color: gray;
        }

        .settingsPage {
            display: none;
        }

        #settingsPage {
            display: none;
            position: fixed;
            z-index: 2;
            width: 100vw;
            height: 100vh;
            background-color: rgba(0, 0, 0, 0.8);
            align-items: flex-start;
        }

        .settings-content {
            background-color: white;
            padding: 10px;
            width: 80em;
            border-radius: 10px;
            max-width: 400px;
            box-shadow: 0 0 10px rgba(0, 0, 0, 0.5);
            text-align: center;
        }

        button {
            background-color: #333;
            font-size: 15px;
            color: white;
            margin-top: 10px;
            border: none;
            padding: 5px;
            align-self: center;
            cursor: pointer;
            border-radius: 5px;
        }

        button:hover {
            background-color: #555;
        }

        .slider-container {
            margin-top: 20px;
        }

        input[type="range"] {
            margin-top: 10px;
        }

        label {
            display: block;
            margin-top: 10px;
        }

        .version-info {
            margin-top: 20px;
            text-align: center;
            font-size: 12px;
            color: #666;
        }

        .status-box {
            text-align: left;
            margin-top: 20px;
            padding: 10px;
            background-color: #f5f5f5;
            border-radius: 10px;
            box-shadow: 0 0 10px rgba(0, 0, 0, 0.5);
        }

        .status-box h4 {
            font-size: 16px;
            text-align: center;
            margin-bottom: 10px;
            margin-top: 0px;
        }

        .status-box p {
            margin: 0;
        }

        .status-box strong {
            font-weight: bold;
        }

        .timer-container {
            margin-top: 20px;
        }

        label[for="timerDelay"] {
            display: block;
            margin-top: 10px;
        }

        #timerDelay {
            padding: 8px;
            font-size: 14px;
            margin-top: 5px;
            border: 1px solid #ccc;
            border-radius: 4px;
            box-sizing: border-box;
        }

    </style>
</head>

<body onload="toggleSettings()">
    <div class="menu-container">
        <div class="menu-icon" onclick="toggleSettings()">Settings</div>
        <h2>Temperature Server</h2>
    </div>
    <div class="settings-page" id="settingsPage">
        <div class="settings-content">
            <h3>Settings</h3>
            <div class="slider-container">
                <label for="minTemperature">Min Temperature:</label>
                <input type="range" id="minTemperature" min="-50" max="50" step="1" value="22" name="minTemperature">
                <span id="minTemperatureValue">22</span>&deg;C
            </div>
            <div class="slider-container">
                <label for="maxTemperature">Max Temperature:</label>
                <input type="range" id="maxTemperature" min="-50" max="50" step="1" value="29" name="maxTemperature">
                <span id="maxTemperatureValue">29</span>&deg;C
            </div>
            <div class="timer-container">
                <label for="timerDelay">Timer Delay:</label>
                <select id="timerDelay" name="timerDelay">
                    <option value="30">30 minutes</option>
                    <option value="45">45 minutes</option>
                    <option value="60">1 hour</option>
                    <option value="90">1 1/2 hours</option>
                    <option value="120">2 hours</option>
                </select>
            </div>
            <button onclick="saveSettings()">Save</button>
            <div class="status-box">
                <h4>Status</h4>
                <p><strong>SSID:</strong> <span id="currentSSID">Loading...</span></p>
                <p><strong>IP Address:</strong> <span id="currentIPAddress">Loading...</span></p>
                <p><strong>Uptime:</strong> <span id="currentUptime">Loading...</span></p>
            </div>
            <button onclick="closeSettings()">Close</button>
            <div class="version-info">
                <p>&copy; 2023 Davin Chiupka.</p>
                <p>Version 1.0.2</p>
            </div>
        </div>
    </div>
    <table id="temperatureTable">
        <thead>
            <tr>
                <th>Celsius</th>
                <th>Fahrenheit</th>
                <th>Time Stamp</th>
            </tr>
        </thead>
        <tbody id="tableBody">
        </tbody>
    </table>
    <script>
        // Function to add a new row to the table
        function addTableRow(celsius, fahrenheit, timestamp, index) {
            var table = document.getElementById("temperatureTable");
            var tbody = table.querySelector("tbody");
            var newRow = tbody.insertRow(-1);

            newRow.classList.add("data-row");
            newRow.setAttribute("data-index", index); // Store the original index

            var cell1 = newRow.insertCell(0);
            var cell2 = newRow.insertCell(1);
            var cell3 = newRow.insertCell(2);

            cell1.innerHTML = '<span class="temperature">' + celsius + '</span>';
            cell2.innerHTML = '<span class="temperature">' + fahrenheit + '</span>';
            cell3.innerHTML = timestamp;
        }

        function updateTable(dataArray) {
            var tbody = document.getElementById("tableBody");
            tbody.innerHTML = '';

            for (var i = 0; i < dataArray.length; i++) {
                if (dataArray[i].temperatureC !== "") {
                    addTableRow(dataArray[i].temperatureC, dataArray[i].temperatureF, dataArray[i].currentTime, i);
                }
            }
        }

        function fetchDataOnce() {
            fetch('/data')
                .then(response => response.json())
                .then(dataArray => {
                    updateTable(dataArray);
                })
                .catch(error => {
                    console.error('Error fetching data:', error);
                });

            fetch('/info')
                .then(response => response.json())
                .then(infoArray => {
                    updateStatus(infoArray);
                })
                .catch(error => {
                    console.error('Error fetching info:', error);
                });
        }

        function fetchDataInterval() {
            setInterval(function() {
                fetch('/data')
                    .then(response => response.json())
                    .then(dataArray => {
                        updateTable(dataArray);
                    })
                    .catch(error => {
                        console.error('Error fetching data:', error);
                    });

                fetch('/info')
                    .then(response => response.json())
                    .then(infoArray => {
                        updateStatus(infoArray);
                    })
                    .catch(error => {
                        console.error('Error fetching info:', error);
                    });
            }, 30500);
        }

        function updateStatus(infoArray) {
            var uptimeSeconds = infoArray[0].UpTime;
            var formattedUptime = formatUptime(uptimeSeconds);

            document.getElementById("currentSSID").innerText = infoArray[0].SSID;
            document.getElementById("currentIPAddress").innerText = infoArray[0].IP;
            document.getElementById("currentUptime").innerText = formattedUptime;
            document.getElementById("minTemperatureValue").innerText = infoArray[0].MinTemp;
            document.getElementById("minTemperatureValue").value = infoArray[0].MinTemp;
            document.getElementById("maxTemperatureValue").innerText = infoArray[0].MaxTemp;
            document.getElementById("maxTemperatureValue").value = infoArray[0].MaxTemp;

            var timerDelaySelect = document.getElementById("timerDelay");
            var selectedOption = Array.from(timerDelaySelect.options).find(option => option.value === infoArray[0].timerDelay);
    
            if (selectedOption) {
              selectedOption.selected = true;
            }
        }

        window.addEventListener('load', function() {
            fetchDataOnce();
            fetchDataInterval();
        });

        function toggleSettings() {
            var settingsPage = document.getElementById("settingsPage");
            settingsPage.style.display = (settingsPage.style.display === "none") ? "flex" : "none";
        }


        function closeSettings() {
            document.getElementById("settingsPage").style.display = "none";
        }

        document.getElementById("minTemperature").addEventListener("input", function() {
            document.getElementById("minTemperatureValue").innerText = this.value;
        });

        document.getElementById("maxTemperature").addEventListener("input", function() {
            document.getElementById("maxTemperatureValue").innerText = this.value;
        });

        function saveSettings() {
            var minTemp = document.getElementById("minTemperature").value;
            var maxTemp = document.getElementById("maxTemperature").value;
            var timerDelayVal = document.getElementById("timerDelay").value;

            updateTemperatureSettings(minTemp, maxTemp, timerDelayVal);
        }

        function updateTemperatureSettings(minTemp, maxTemp, timerDelay) {
            fetch(`/updateSettings?minTemperature=${minTemp}&maxTemperature=${maxTemp}&timerDelay=${timerDelay}`)
                .then(response => {
                    if (!response.ok) {
                        throw new Error(`HTTP error! Status: ${response.status}`);
                    }
                    return response.text();
                })
                .then(data => {
                    console.log('Temperature settings updated:', data);
                })
                .catch(error => {
                    console.error('Error updating temperature settings:', error);
                });
        }

        function formatUptime(seconds) {
            var hours = Math.floor(seconds / 3600);
            var minutes = Math.floor((seconds %% 3600) / 60);
            var remainingSeconds = seconds %% 60;

            return `${hours}h ${minutes}m ${remainingSeconds}s`;
        }

    </script>
</body>

</html>
)rawliteral";
#endif