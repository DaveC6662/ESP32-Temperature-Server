/*
  Header: temper.h
  Author: Davin Chiupka
  Date: December 1, 2023
  Last Updated: January 18th, 2024

  Description:
  This header file provides functionality for interfacing with a DS18B20 temperature sensor
  using the OneWire and DallasTemperature libraries. It includes constants, variables, data structures,
  an array for storing temperature data, and functions for reading temperature values.

  Includes:
  - OneWire: Library for communication with 1-wire devices, such as the DS18B20 temperature sensor.
  - DallasTemperature: Library for the DS18B20 temperature sensor.

  Usage:
  - Include this header file in your main Arduino sketch to access temperature-related functionality.
  - Adjust MAX_TEMP and MIN_TEMP for initial temperature alert thresholds.

  Notes:
  - Ensure proper wiring of the DS18B20 sensor to the specified pin (ONE_WIRE_BUS).
  - Calibration may be required for accurate temperature readings.
*/

#ifndef TEMPER_H
#define TEMPER_H

// Define the GPIO pin number for the OneWire data bus (used for DS18B20 temperature sensor).
#define ONE_WIRE_BUS 4

// Create an instance of the OneWire object to communicate with OneWire devices.
OneWire oneWire(ONE_WIRE_BUS);

// Create an instance of the DallasTemperature library to interface with DS18B20 sensors.
DallasTemperature sensors(&oneWire);

// Maximum temperature threshold for alerting (in Celsius).
float MAX_TEMP = 25.0;

// Minimum temperature threshold for alerting (in Celsius).
float MIN_TEMP = 22.0;

// Variables to store the latest temperature readings as strings.
String temperatureF = ""; // Fahrenheit
String temperatureC = ""; // Celsius
String currentTime = "";  // Current time when the reading was taken

// Structure to store temperature data (Celsius, Fahrenheit) and the corresponding time.
struct TemperatureData {
  String temperatureC;    // Temperature in Celsius
  String temperatureF;    // Temperature in Fahrenheit
  String currentTime;     // Time of the temperature reading
};

// Maximum number of rows in the temperature data array.
const int MAX_ROWS = 288;

// Array to store temperature data readings.
TemperatureData temperatureArray[MAX_ROWS];

// Index to track the current position in the temperatureArray for the next reading.
int Temp_Array_Index = 0;

/**
 * Reads the temperature in Celsius from a DS18B20 sensor.
 * 
 * This function initiates a temperature reading request to the DS18B20 sensor
 * and retrieves the temperature in Celsius. The temperature is read from the first 
 * (index 0) sensor on the bus.
 * 
 * If the sensor does not return a valid reading (indicated by a value of -127.00),
 * the function returns a placeholder string "--" to indicate an error or unavailable reading.
 * Otherwise, it returns the temperature as a string.
 * 
 * @return A string representing the temperature in Celsius. Returns "--" if the sensor reading is invalid.
 */
String readDSTemperatureC() {
  sensors.requestTemperatures();
  float tempC = sensors.getTempCByIndex(0);

  if (tempC == -127.00) {
    return "--";
  } 
  return String(tempC);
}

/**
 * Reads the temperature in Fahrenheit from a DS18B20 sensor.
 * 
 * This function sends a request to the DS18B20 sensor to measure the temperature
 * and retrieves the temperature in Fahrenheit. It reads the temperature from the first 
 * sensor on the bus (index 0).
 * 
 * In cases where the sensor returns an invalid reading, which is indicated by a 
 * temperature of -196 degrees Fahrenheit (equivalent to the error value of -127.00 
 * degrees Celsius in the sensor's reading), the function returns a placeholder string "--".
 * This indicates an error or that the reading is unavailable. Otherwise, it returns 
 * the temperature as a string.
 * 
 * @return A string representing the temperature in Fahrenheit. Returns "--" if the sensor reading is invalid.
 */
String readDSTemperatureF() {
  sensors.requestTemperatures();
  float tempF = sensors.getTempFByIndex(0);

  if (int(tempF) == -196) {
    return "--";
  } 
  return String(tempF);
}

/**
 * Processes template placeholders for dynamic web content.
 * 
 * This function is used for handling dynamic placeholders in HTML templates. It replaces
 * specific placeholders with corresponding variable values. 
 * 
 * The function checks the provided 'var' argument against known placeholders and returns
 * the appropriate value as a string. Currently supported placeholders include:
 * - "TEMPERATUREC": Returns the current temperature in Celsius.
 * - "TEMPERATUREF": Returns the current temperature in Fahrenheit.
 * - "CURRENTTIME": Returns the current time.
 * 
 * If the placeholder does not match any of the known ones, an empty string is returned.
 * 
 * @param var The placeholder string to be processed.
 * @return A string with the value corresponding to the placeholder, or an empty string if not found.
 */
String processor(const String& var){
  if(var == "TEMPERATUREC"){
    return temperatureC;
  }
  else if(var == "TEMPERATUREF"){
    return temperatureF;
  }
  else if(var == "CURRENTTIME"){
    return currentTime;
  }
  return String();
}

#endif