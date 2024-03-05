#include <Arduino.h>
#include <ESP8266WiFi.h>
#include <Firebase_ESP_Client.h>
#include "addons/TokenHelper.h"
#include "addons/RTDBHelper.h"

//Provide the token generation process info.
#include "addons/TokenHelper.h"
//Provide the RTDB payload printing info and other helper functions.
#include "addons/RTDBHelper.h"

// Insert your network credentials
#define WIFI_SSID "bangg?"
#define WIFI_PASSWORD "hwvv1212"

// Insert Firebase project API Key
#define API_KEY "AIzaSyBnPADOix0EWKu29eysInzDlbA5Rvv9Lz8"

// Insert RTDB URLefine the RTDB URL */
#define DATABASE_URL "https://bismillah-ambil-data-beneran-default-rtdb.asia-southeast1.firebasedatabase.app" 

//Define Firebase Data object
FirebaseData fbdo;

FirebaseAuth auth;
FirebaseConfig config;

unsigned long sendDataPrevMillis = 0;
bool signupOK = false;
int mq4sensor = A0;
int sensorvalue;
String firestatus = "";

void setup() {
  Serial.begin(9600);
  pinMode(mq4sensor, INPUT); // MQ4 sensor
  Serial.println(F("\nESP8266 WiFi scan example"));

  // Set WiFi to station mode
  WiFi.mode(WIFI_STA);

  // Disconnect from an AP if it was previously connected
  WiFi.disconnect();
  delay(100);

  WiFi.begin(WIFI_SSID, WIFI_PASSWORD);
  Serial.print("Connecting to Wi-Fi");
  while (WiFi.status() != WL_CONNECTED) {
    Serial.print(".");
    delay(300);
  }
  Serial.println();
  Serial.print("Connected with IP: ");
  Serial.println(WiFi.localIP());
  Serial.println();

  /* Assign the api key (required) */
  config.api_key = API_KEY;

  /* Assign the RTDB URL (required) */
  config.database_url = DATABASE_URL;

  /* Sign up */
  if (Firebase.signUp(&config, &auth, "", "")) {
    Serial.println("ok");
    signupOK = true;
  } else {
    Serial.printf("%s\n", config.signer.signupError.message.c_str());
  }

  /* Assign the callback function for the long running token generation task */
  config.token_status_callback = tokenStatusCallback; //see addons/TokenHelper.h

  Firebase.begin(&config, &auth);
  Firebase.reconnectWiFi(true);
}

void loop() {
  int32_t rssi;
  int scanResult;

  Serial.println(F("Scanning WiFi..."));

  scanResult = WiFi.scanNetworks(/*async=*/false, /*hidden=*/true);

  if (scanResult > 0) {
    
    // Find the specified WiFi network in the scan results
    for (int8_t i = 0; i < scanResult; i++) {
      String ssid = WiFi.SSID(i);
      if (ssid.equals(WIFI_SSID)) {
        rssi = WiFi.RSSI(i); // Get RSSI directly

        Serial.printf(PSTR("WiFi network found: %s\n"), ssid.c_str());
        
        // Display RSSI
        Serial.printf(PSTR("RSSI: %d dBm\n"), rssi);

        // Send RSSI to Firebase
        if (Firebase.ready() && signupOK) {
          Firebase.RTDB.setFloat(&fbdo, "WiFi/RSSI", rssi);
        }

        break; // Stop scanning once the specified network is found
      }
    }
  } else {
    Serial.println(F("No WiFi networks found"));
  }

  // Wait a bit before scanning again
  delay(5000);

  if (Firebase.ready() && signupOK && (millis() - sendDataPrevMillis > 5000 || sendDataPrevMillis == 0)) {
    sendDataPrevMillis = millis();
    sensorvalue = analogRead(mq4sensor); // read the MQ4 sensor
    Serial.println("MQ4: " + String(sensorvalue));
    Firebase.RTDB.setFloat(&fbdo, "MQ4 Sensor/Value", sensorvalue);
  }
}