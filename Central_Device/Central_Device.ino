#include <WiFi.h>
#include <HTTPClient.h>
#include <BLEDevice.h>
#include <BLEUtils.h>
#include <BLEScan.h>
#include <BLEClient.h>

#define WIFI_SSID "a"              // Replace with your WiFi SSID
#define WIFI_PASSWORD "12344321"   // Replace with your WiFi password
#define SERVER_URL "http://192.168.38.206:5000/api/data"  // Replace with your server's IP address and endpoint

#define SERVICE_UUID "df67ff1a-718f-11e7-8cf7-a6006ad3dba0"
#define CHARACTERISTIC_UUID "01000000-0000-0000-0000-000000efcdab"

int lastRssiValue = 0;
BLEScan* pBLEScan;
BLEClient* pClient;

bool isConnected = false;
unsigned long connectionStartTime = 0;
unsigned long rssiCheckInterval = 5000;
unsigned long lastRssiCheckTime = 0;
const int rssiThreshold = -80;

// Define arrays to store RSSI values and connection durations
const int maxEntries = 500;
int rssiValues[maxEntries];
unsigned long connectionDurations[maxEntries];
int currentIndex = 0;
  
void connectToWiFi() {
    Serial.print("Connecting to WiFi...");
    WiFi.begin(WIFI_SSID, WIFI_PASSWORD);
    while (WiFi.status() != WL_CONNECTED) {
        delay(1000);
        Serial.print(".");
    }
    Serial.println("\nConnected to WiFi.");
}

void postData() {
    if (WiFi.status() == WL_CONNECTED) {
        HTTPClient http;
        http.begin(SERVER_URL);
        http.addHeader("Content-Type", "application/json");

        // Construct JSON payload
        String jsonPayload = "{ \"rssi_values\": [";
        for (int i = 0; i < currentIndex; i++) {
            jsonPayload += String(rssiValues[i]);
            if (i < currentIndex - 1) jsonPayload += ", ";
        }
        jsonPayload += "], \"last_retention\": " + String(connectionDurations[currentIndex - 1]) + " }";

        int httpResponseCode = http.POST(jsonPayload);

        if (httpResponseCode > 0) {
            String response = http.getString();
            Serial.print("Server response: ");
            Serial.println(response);
        } else {
            Serial.print("Error in HTTP request: ");
            Serial.println(http.errorToString(httpResponseCode).c_str());
        }

        http.end();
    } else {
        Serial.println("WiFi not connected. Cannot send data.");
    }
}

void setup() {
    Serial.begin(115200);
    connectToWiFi();

    BLEDevice::init("ESP32 Central Device");
    pBLEScan = BLEDevice::getScan();
    pBLEScan->setActiveScan(true);
    pBLEScan->setInterval(100);
    pBLEScan->setWindow(99);
    startScanning();
}

void startScanning() {
    Serial.println("Starting BLE Scan...");
    pBLEScan->start(5, false);
}

void connectToServer(BLEAddress address) {
    Serial.print("Attempting to connect to device: ");
    Serial.println(address.toString().c_str());

    pClient = BLEDevice::createClient();
    if (!pClient->connect(address)) {
        Serial.println("Failed to connect to device.");
        return;
    }
    Serial.println("Connected to device.");

    isConnected = true;
    connectionStartTime = millis();
    currentIndex = 0; // Reset the current index when connected
}

void checkRSSI() {
    if (isConnected && millis() - lastRssiCheckTime >= rssiCheckInterval) {
        lastRssiCheckTime = millis();
        lastRssiValue = pClient->getRssi();
        Serial.print("Current RSSI Value: ");
        Serial.println(lastRssiValue);

        // Store only if we have space and the value is new or changed
        if (currentIndex < maxEntries) {
            rssiValues[currentIndex] = lastRssiValue;
            connectionDurations[currentIndex] = (millis() - connectionStartTime) / 1000;
            currentIndex++;
        }

        if (lastRssiValue < rssiThreshold) {
            Serial.println("Device is out of range. Disconnecting...");
            pClient->disconnect();
            isConnected = false;

            postData(); // Post data to server after disconnection
            startScanning();
        }
    }
}

void loop() {
    if (isConnected) {
        if (!pClient->isConnected()) {
            Serial.println("Disconnected from device.");
            isConnected = false;
            postData(); // Post data when disconnected
            startScanning();
        } else {
            checkRSSI();
            return;
        }
    } else {
        startScanning();
        delay(1000);

        BLEScanResults* foundDevices = pBLEScan->getResults();
        for (int i = 0; i < foundDevices->getCount(); i++) {
            BLEAdvertisedDevice device = foundDevices->getDevice(i);
            if (device.haveServiceUUID() && device.getServiceUUID().toString() == SERVICE_UUID) {
                connectToServer(device.getAddress());
                break;
            }
        }
    }
}
