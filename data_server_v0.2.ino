/*
  Program: data_server.ino
  Version: 1.0.3
  Author: Davin Chiupka
  Created: December 3rd, 2023
  Last Updated: January 18th, 2024

  Description:
    This program is designed for ESP32 boards to capture temperature data from a DS18B20 sensor and send it to a self hosted server.
    It also notifies via webhooks when the temperature exceeds a set maximum or falls below a set minimum.

  features:
    - WiFi connectivity with both WPA2-Personal and WPA2-Enterprise support.
    - Web server for real-time data presentation and configuration.
    - Webhook integration for temperature alerts.
    - Configurable temperature thresholds and timer delay for alerts.

  Usage Instructions:
    1. Connect the DS18B20 sensor to the ESP32.
    2. Set WiFi and webhook configurations in "secrets.h".
    3. Compile and upload the program to an ESP32 board.
    4. Use the AP mode for initial WiFi configuration.
    5. Access the web server for real-time data and settings.

  Hardware Required:
    - ESP32 board
    - DS18B20 temperature sensor
    - 4.7k resistor

  Libraries Used:
    - DNSServer: For creating a DNS server on the ESP32.
    - WiFi: To manage WiFi connectivity.
    - AsyncTCP: Dependency for ESPAsyncWebServer.
    - ESPAsyncWebServer: To create an asynchronous web server.
    - OneWire: Interface with DS18B20 sensor.
    - DallasTemperature: Manage the DS18B20 temperature readings.
    - esp_wpa2: For WPA2-Enterprise WiFi security.
    - HTTPClient: To send HTTP requests (for webhooks).
    - ArduinoJson: For JSON serialization and parsing.

  Additional Files:
    - secrets.h: Contains sensitive configuration like WiFi credentials and webhook URLs.
    - html.h: HTML source for the web server pages.
    - temper.h: Includes temperature-related functions and constants.

  Note:
    Ensure all library dependencies are installed before compiling. Follow best practices for secure handling of WiFi credentials and webhook URLs.
*/

#include <DNSServer.h>
#include <WiFi.h>
#include <AsyncTCP.h>
#include "ESPAsyncWebSrv.h"
#include <OneWire.h>
#include <DallasTemperature.h>
#include "esp_wpa2.h"
#include <HTTPClient.h>
#include <ArduinoJson.h>
#include <ArduinoJson.hpp>

#include "secrets.h"
#include "html.h"
#include "temper.h"

// Counter used in various loops for retry mechanisms or timing.
int counter = 0;

// Stores the last time (in milliseconds) a certain action was performed.
unsigned long lastTime = 0;

// Delay (in milliseconds) used for a timer, e.g., for temperature reading intervals.
unsigned long timerDelay = 300000;  // 5 minutes

// Stores the last time (in milliseconds) a notification was sent.
unsigned long lastNotifyTime = 0;

// Delay (in milliseconds) before sending another notification to prevent spam.
unsigned long teamsNotificationDelay = 1800000; // 30 minutes

// Flag to indicate whether a notification has been sent.
bool sendNotif = false;

// NTP server for time synchronization.
const char* ntpServer = "pool.ntp.org";

// GMT offset for local time calculation in seconds.
const long gmtOffset_sec = -18000; // Offset for Eastern Standard Time (EST)

// Daylight saving time offset in seconds.
const int daylightOffset_sec = 3600; // Typically 1 hour

// DNS server for the captive portal.
DNSServer dnsServer;

// Web server running on port 80.
AsyncWebServer server(80);

// WiFi Enterprise Authentication Identity.
String EAP_IDENTITY = "";

// WiFi Enterprise Authentication Password.
String EAP_PASSWORD = "";

// WiFi network SSID to connect to.
String ssid = "";

// WiFi network password.
String password = "";

// Type of WiFi security (e.g., WPA2-Personal, WPA2-Enterprise).
String security = "";

// Passcode for additional security or configuration.
String passcode = "";

// Flags to indicate if WiFi credentials have been received.
bool ssid_received = false;
bool username_received = false;
bool passcode_received = false;
bool password_received = false;
bool security_received = false;

// Flag to indicate if the system is waiting to connect to WiFi.
bool waiting_to_connect = true;

// Flag indicating whether the device is in WiFi configuration mode.
bool configuringWiFi = true;

/**
 * Converts milliseconds to minutes.
 *
 * This function takes a time duration in milliseconds and converts it to minutes.
 *
 * @param milliseconds The time duration in milliseconds.
 * @return The equivalent time duration in minutes.
 */
int millisecondsToMinutes(long milliseconds) {
  return milliseconds / (1000 * 60);
}

/**
 * Converts minutes to milliseconds.
 *
 * This function takes a time duration in minutes and converts it to milliseconds.
 *
 * @param minutes The time duration in minutes.
 * @return The equivalent time duration in milliseconds.
 */
long minutesToMilliseconds(int minutes) {
  return minutes * 60 * 1000;
}

/**
 * Captive Request Handler for an Async Web Server.
 *
 * This class is a custom handler for the ESPAsyncWebServer, designed to manage HTTP requests
 * during the captive portal phase of the ESP32's operation. It inherits from AsyncWebHandler,
 * a base class for handling web requests asynchronously.
 *
 * The primary function of this handler is to respond to all HTTP requests with either the
 * data page or the index page, depending on whether the ESP32 is still waiting to connect
 * to the WiFi or is already connected.
 *
 * Public Methods:
 * - CaptiveRequestHandler(): Constructor for initializing the handler.
 * - ~CaptiveRequestHandler(): Virtual destructor.
 * - canHandle(AsyncWebServerRequest *request): Determines if this handler can process the incoming request.
 * - handleRequest(AsyncWebServerRequest *request): Handles the incoming web requests.
 *
 * Usage:
 * Attach this handler to an ESPAsyncWebServer instance to manage requests during the WiFi setup phase.
 */
class CaptiveRequestHandler : public AsyncWebHandler {
public:
  CaptiveRequestHandler() {}
  virtual ~CaptiveRequestHandler() {}

  bool canHandle(AsyncWebServerRequest* request) {
    return true;
  }

  void handleRequest(AsyncWebServerRequest* request) {
    if (!waiting_to_connect) {
      request->send_P(200, "text/html", data_html);
    }
    else {
      request->send_P(200, "text/html", index_html);
    }
  }
};

/**
 * Retrieves the current local time as a formatted string.
 *
 * This function configures the time for the ESP32 using the NTP server specified
 * by 'ntpServer' global variable, along with the GMT offset and daylight saving
 * time offset. It then retrieves the current local time and formats it into a
 * human-readable string format.
 *
 * If the function fails to obtain the local time (for example, due to a failure
 * in connecting to the NTP server), it returns a placeholder string "--".
 *
 * @return A string representing the current local time formatted as
 *         "Weekday, Month Day Year Hour:Minute". If the time cannot be retrieved,
 *         returns "--".
 */
String getLocalTime() {
  struct tm timeinfo;

  configTime(gmtOffset_sec, daylightOffset_sec, ntpServer);

  if (!getLocalTime(&timeinfo)) {
    return "--";
  }

  char timeString[40];
  strftime(timeString, sizeof(timeString), "%A, %B %d %Y %H:%M", &timeinfo);
  return String(timeString);
}

/**
 * Sets up and configures the web server routes and handlers.
 *
 * This function initializes the routes and handlers for the ESPAsyncWebServer instance.
 * It defines the behavior for various endpoints, such as "/", "/data", "/info", and "/updateSettings".
 * Each endpoint is configured to handle GET requests and respond appropriately based on the current state of the system
 * (e.g., whether the device is waiting to connect to WiFi) and the incoming request parameters.
 *
 * - The root ("/") route serves the main HTML page. It serves 'data_html' if the device is connected, or 'index_html' if it's in the setup phase.
 * - The "/data" route provides temperature data in JSON format.
 * - The "/info" route provides device and configuration info in JSON format.
 * - The "/updateSettings" route allows updating temperature thresholds and notification delay via query parameters.
 * - The "/get" route is used during the WiFi configuration phase to receive WiFi settings from the user.
 */
void setupServer() {
  server.on("/", HTTP_GET, [](AsyncWebServerRequest* request) {
    if (!waiting_to_connect) {
      request->send_P(200, "text/html", data_html, processor);
    }
    else {
      request->send_P(200, "text/html", index_html);
    }
    });

  if (!waiting_to_connect) {
    server.on("/data", HTTP_GET, [](AsyncWebServerRequest* request) {
      String json = "[";
      for (int i = 0; i < MAX_ROWS; i++) {
        json += "{\"temperatureC\":\"" + temperatureArray[i].temperatureC + "\",\"temperatureF\":\"" + temperatureArray[i].temperatureF + "\",\"currentTime\":\"" + temperatureArray[i].currentTime + "\"}";
        if (i < MAX_ROWS - 1) {
          json += ",";
        }
      }
      json += "]";
      request->send(200, "application/json", json);
      });

    server.on("/info", HTTP_GET, [](AsyncWebServerRequest* request) {
      int minTempInt = int(MIN_TEMP);
      int maxTempInt = int(MAX_TEMP);
      int timerDelay = millisecondsToMinutes(teamsNotificationDelay);
      String json = "[{\"SSID\":\"" + WiFi.SSID() + "\",\"IP\":\"" + WiFi.localIP().toString() + "\",\"UpTime\":\"" + (millis() / 1000) + "\",\"MinTemp\":\"" + minTempInt + "\",\"MaxTemp\":\"" + maxTempInt + "\",\"timerDelay\":\"" + timerDelay + "\"}]";
      request->send(200, "application/json", json);
      });

    server.on("/updateSettings", HTTP_GET, [](AsyncWebServerRequest* request) {
      MIN_TEMP = request->getParam("minTemperature")->value().toFloat();
      MAX_TEMP = request->getParam("maxTemperature")->value().toFloat();
      int notifDelay = request->getParam("timerDelay")->value().toInt();
      teamsNotificationDelay = minutesToMilliseconds(notifDelay);
      request->send(200, "text/plain", "Settings updated successfully");
      });
  }

  else {

    server.on("/get", HTTP_GET, [](AsyncWebServerRequest* request) {
      String inputMessage;
      String inputParam;

      if (request->hasParam("Security")) {
        inputMessage = request->getParam("Security")->value();
        inputParam = "Security";
        security = inputMessage;
        security_received = true;
      }

      if (request->hasParam("SSID")) {
        inputMessage = request->getParam("SSID")->value();
        inputParam = "SSID";
        ssid = inputMessage;
        ssid_received = true;
      }

      if (request->hasParam("Username")) {
        inputMessage = request->getParam("Username")->value();
        inputParam = "Username";
        EAP_IDENTITY = inputMessage;
        username_received = true;
      }

      if (request->hasParam("Passcode")) {
        inputMessage = request->getParam("Passcode")->value();
        inputParam = "Passcode";
        passcode = inputMessage;
        if (passcode == SECRET_PASSCODE) {
          passcode_received = true;
        }
      }

      if (security == "WPA2-Personal") {
        if (request->hasParam("Password")) {
          inputMessage = request->getParam("Password")->value();
          inputParam = "Password";
          password = inputMessage;
          password_received = true;
        }
      }
      else if (security == "WPA2-Enterprise") {
        if (request->hasParam("Password")) {
          inputMessage = request->getParam("Password")->value();
          inputParam = "Password";
          EAP_PASSWORD = inputMessage;
          password_received = true;
        }
      }
      });
  }
}

/**
 * Initializes and connects the ESP32 to a WiFi network.
 *
 * This function configures the WiFi settings for the ESP32 and attempts to connect
 * to a WiFi network. It supports both WPA2-Personal and WPA2-Enterprise security.
 *
 * The function disconnects any existing WiFi connections, sets the WiFi mode to STA (Station),
 * and then initiates a connection using the credentials input into the captive portal.
 *
 * For WPA2-Enterprise, it sets the identity, username, and password before starting the connection.
 *
 * The function includes a retry mechanism that attempts to connect to the WiFi network
 * multiple times (up to 60 retries, with a 500ms delay between each attempt). If the ESP32
 * cannot connect to the WiFi network within these attempts, it restarts itself.
 *
 * Once connected to the WiFi, the function calls 'infoWebHook' to send device information,
 * initializes the sensors, and sets up the web server for handling HTTP requests.
 */
void setupWiFi() {

  WiFi.disconnect(true);
  WiFi.mode(WIFI_STA);

  int counter = 0;

  if (security == "WPA2-Personal") {
    WiFi.begin(ssid.c_str(), password.c_str());
  }
  else if (security == "WPA2-Enterprise") {
    esp_wifi_sta_wpa2_ent_set_identity((uint8_t*)EAP_IDENTITY.c_str(), EAP_IDENTITY.length());
    esp_wifi_sta_wpa2_ent_set_username((uint8_t*)EAP_IDENTITY.c_str(), EAP_IDENTITY.length());
    esp_wifi_sta_wpa2_ent_set_password((uint8_t*)EAP_PASSWORD.c_str(), EAP_PASSWORD.length());
    esp_wifi_sta_wpa2_ent_enable();
    WiFi.begin(ssid.c_str());
  }

  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
    counter++;
    if (counter >= 60) {
      ESP.restart();
    }
  }
  waiting_to_connect = false;

  infoWebHook();

  EAP_IDENTITY = "";
  EAP_PASSWORD = "";
  password = "";
  passcode = "";

  sensors.begin();
  currentTime = getLocalTime();
  temperatureC = readDSTemperatureC();
  temperatureF = readDSTemperatureF();

  temperatureArray[Temp_Array_Index].temperatureC = temperatureC;
  temperatureArray[Temp_Array_Index].temperatureF = temperatureF;
  temperatureArray[Temp_Array_Index].currentTime = currentTime;
  Temp_Array_Index++;

  setupServer();
}

/**
 * Sends temperature data to a specified webhook URL when temperature thresholds are exceeded.
 *
 * This function is designed to send an HTTP POST request to a webhook URL (defined in 'TEMP_WEBHOOK_URL')
 * with temperature data in JSON format. It is triggered when the temperature readings fall outside
 * the predefined minimum and maximum thresholds (MIN_TEMP and MAX_TEMP) or the set values in the
 * web servers settings page.
 *
 * The temperature data, along with the current time, is sent in a JSON payload.
 *
 * @param tempC The current temperature in Celsius, as a String.
 * @param tempF The current temperature in Fahrenheit, as a String.
 * @param time The current time, typically when the temperature reading was taken, as a String.
 *
 * * Notes:
 * - The webhook URL ('TEMP_WEBHOOK_URL') should be predefined and correctly set to receive the JSON data.
 */
void tempWebHook(String tempC, String tempF, String time) {
  float temperatureCFloat = tempC.toFloat();
  if (temperatureCFloat > MAX_TEMP || temperatureCFloat < MIN_TEMP) {
    String url = TEMP_WEBHOOK_URL;
    HTTPClient http;
    http.begin(url);
    http.addHeader("Content-Type", "application/json");
    String data = "{";
    data += "\"temperatureC\": \"" + tempC + "\",";
    data += "\"temperatureF\": \"" + tempF + "\",";
    data += "\"time\": \"" + time + "\",";
    data += "\"minTemp\": \"" + String(MIN_TEMP) + "\",";
    data += "\"maxTemp\": \"" + String(MAX_TEMP) + "\"";
    data += "}";int httpResponseCode = http.POST(data);
    if (httpResponseCode > 0) {
      String response = http.getString();
    }
    http.end();
  }
}

/**
 * Sends device connection information to a specified webhook URL.
 *
 * This function gathers information about the ESP32's network connection, including
 * the MAC address, IP address, and connected SSID. For WPA2-Enterprise connections, it also includes
 * the user login (EAP_IDENTITY). This information is then sent as a JSON payload via an HTTP POST
 * request to the webhook URL defined in 'INFO_WEBHOOK_URL'.
 *
 * The function is useful for remotely monitoring the network status and identity of the device,
 * especially after initial configuration or during routine operations.
 *
 * Notes:
 * - The function checks the security protocol (WPA2-Personal or WPA2-Enterprise) to determine
 *   the relevant information to include in the payload.
 * - The webhook URL ('INFO_WEBHOOK_URL') should be predefined and correctly set to receive the JSON data.
 */
void infoWebHook() {

  String macAddress = WiFi.macAddress();
  String ipAddress = WiFi.localIP().toString();
  String connectedSSID = WiFi.SSID();


  String url = INFO_WEBHOOK_URL;
  HTTPClient http;
  http.begin(url);
  http.addHeader("Content-Type", "application/json");

  String data = "{";
  data += "\"macAddress\": \"" + macAddress + "\",";
  data += "\"ipAddress\": \"" + ipAddress + "\",";
  data += "\"connectedSSID\": \"" + connectedSSID + "\"";

  if (security == "WPA2-Enterprise") {
    data += ", \"userLogin\": \"" + EAP_IDENTITY + "\"";
  }
  data += "}";
  int httpResponseCode = http.POST(data);

  if (httpResponseCode > 0) {
    String response = http.getString();
  }

  http.end();
}

/**
 * Initial setup function for the ESP32 device.
 *
 * This function is called once when the ESP32 starts. It performs the initial setup of the device,
 * including starting the serial communication, setting up the WiFi in Access Point (AP) mode,
 * initializing the DNS server, and setting up the web server with the necessary handlers.
 *
 * The ESP32 is configured to create its own WiFi network ("Connect AP"), allowing for initial configuration
 * through a captive portal. This is particularly useful for setting up the device in new environments
 * where WiFi credentials need to be configured.
 */
void setup() {
  Serial.begin(115200);
  WiFi.disconnect(true);
  WiFi.mode(WIFI_AP);
  WiFi.softAP("Connect AP");
  setupServer();
  dnsServer.start(53, "*", WiFi.softAPIP());
  server.addHandler(new CaptiveRequestHandler()).setFilter(ON_AP_FILTER);
  server.begin();
}

/**
 * Main loop function for the ESP32 device.
 *
 * This function continuously runs after the initial setup. It performs several key tasks:
 * 1. Processes DNS requests when in AP mode for captive portal functionality.
 * 2. Once the WiFi credentials are received, it attempts to connect to the WiFi network.
 * 3. Regularly checks and reads the temperature from the sensors.
 * 4. Sends temperature data to a webhook if the temperature is outside the predefined thresholds.
 * 5. Manages the storage of temperature data in a circular array.
 *
 * The function ensures continuous monitoring of the temperature and the device's connectivity status,
 * triggering actions like sending alerts or storing temperature data based on the defined conditions.
 */
void loop() {
  dnsServer.processNextRequest();
  if (ssid_received && password_received && passcode_received) {
    while (waiting_to_connect) {
      setupWiFi();
    }
  }

  if (((millis() - lastTime) > timerDelay) && !waiting_to_connect) {

    temperatureC = readDSTemperatureC();
    temperatureF = readDSTemperatureF();
    currentTime = getLocalTime();

    float temperatureCFloat = temperatureC.toFloat();
    if ((temperatureCFloat < MIN_TEMP || temperatureCFloat > MAX_TEMP) && !sendNotif) {

      tempWebHook(temperatureC, temperatureF, currentTime);
      lastNotifyTime = millis();
      sendNotif = true;
    }

    if ((millis() - lastNotifyTime) > teamsNotificationDelay && sendNotif) {
      tempWebHook(temperatureC, temperatureF, currentTime);
      lastNotifyTime = millis();
    }

    temperatureArray[Temp_Array_Index].temperatureC = temperatureC;
    temperatureArray[Temp_Array_Index].temperatureF = temperatureF;
    temperatureArray[Temp_Array_Index].currentTime = currentTime;
    Temp_Array_Index = (Temp_Array_Index + 1) % MAX_ROWS;
    lastTime = millis();
  }
}
