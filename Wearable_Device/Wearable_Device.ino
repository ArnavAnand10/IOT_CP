#include <bluefruit.h>

// Custom Service and Characteristic UUIDs
const uint8_t SERVICE_UUID[] = {
  0xA0, 0xDB, 0xD3, 0x6A, 0x00, 0xA6, 0xF7, 0x8C,
  0xE7, 0x11, 0x8F, 0x71, 0x1A, 0xFF, 0x67, 0xDF
};

const uint8_t CHARACTERISTIC_UUID[] = {
  0xAB, 0xCD, 0xEF, 0x00, 0x00, 0x00, 0x00, 0x00,
  0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x01
};

// Create BLE service and characteristic objects
BLEService customService = BLEService(SERVICE_UUID);
BLECharacteristic customCharacteristic = BLECharacteristic(CHARACTERISTIC_UUID);

// Variable to store data
int counter = 0;

void setup() {
  Serial.begin(115200);
  while (!Serial); // Wait for Serial to be ready

  // Initialize Bluefruit
  Bluefruit.begin();
  Bluefruit.setTxPower(4); // Set transmit power

  // Set connection and disconnection callbacks
  Bluefruit.Periph.setConnectCallback(connect_callback);
  Bluefruit.Periph.setDisconnectCallback(disconnect_callback);
  Bluefruit.autoConnLed(false); // Disable connection LED for lower power

  // Add custom service and characteristic
  customService.begin();
  customCharacteristic.setProperties(CHR_PROPS_READ | CHR_PROPS_NOTIFY);
  customCharacteristic.setPermission(SECMODE_OPEN, SECMODE_NO_ACCESS);
  customCharacteristic.setFixedLen(4); // Length for integer data
  customCharacteristic.begin();

  // Start advertising
  startAdvertising();
  Serial.println("Peripheral setup complete. Advertising...");
}

void startAdvertising() {
  Bluefruit.Advertising.clearData();
  Bluefruit.Advertising.addFlags(BLE_GAP_ADV_FLAGS_LE_ONLY_GENERAL_DISC_MODE);
  Bluefruit.Advertising.addTxPower();
  Bluefruit.Advertising.addService(customService);
  Bluefruit.Advertising.restartOnDisconnect(true);
  Bluefruit.Advertising.setInterval(32, 244); // Advertising interval in units of 0.625 ms
  Bluefruit.Advertising.start(0); // Advertise indefinitely
}

void connect_callback(uint16_t conn_handle) {
  Serial.println("Connected");
}

void disconnect_callback(uint16_t conn_handle, uint8_t reason) {
  Serial.println("Disconnected");
}

void loop() {
  // Update and notify characteristic value every second
  counter++;
  customCharacteristic.write32(counter); // Write 32-bit integer value  
  customCharacteristic.notify32(counter); // Notify central of new value

  Serial.print("Value sent: ");
  Serial.println(counter);    

  delay(1000); // Wait 1 second
}
