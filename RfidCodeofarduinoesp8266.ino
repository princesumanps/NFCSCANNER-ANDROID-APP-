/* Read RFID Tag with RC522 RFID Reader
 *  Made by miliohm.com
 */
// #include<bits/stdc++.h> 
#include <SPI.h>
#include <MFRC522.h>
#include <Firebase_ESP_Client.h>


/* 1. Define the WiFi credentials */
#define WIFI_SSID "ADD_YOUR_WIFI_NAME"
#define WIFI_PASSWORD "ADD_YOUR_WIFI_PASSWORD"

/* 2. Define the API Key */
#define API_KEY "ADD_YOUR_API_KEY_OF_FIREBASE"

/* 3. Define the RTDB URL */
#define DATABASE_URL "ADD_YOUR_FIREBASE_DATABASE_URL" 

/* 4. Define the user Email and password that alreadey registerd or added in your project */
#define USER_EMAIL "ADD_YOUR_EMAIL"
#define USER_PASSWORD "ADD_PASSWORD"

// Define Firebase Data object
FirebaseData fbdo;

FirebaseAuth auth;
FirebaseConfig config;

unsigned long sendDataPrevMillis = 0;


constexpr uint8_t RST_PIN = 0;     // Configurable, see typical pin layout above
constexpr uint8_t SS_PIN = 2;     // Configurable, see typical pin layout above

MFRC522 rfid(SS_PIN, RST_PIN); // Instance of the class
MFRC522::MIFARE_Key key;

String tag;
double num;

void setup() {
  Serial.begin(115200);
   SPI.begin(); // Init SPI bus
  rfid.PCD_Init(); // Init MFRC522
  WiFi.begin(WIFI_SSID, WIFI_PASSWORD);

  Serial.print("Connecting to Wi-Fi");
  while (WiFi.status() != WL_CONNECTED)
  {
    Serial.print(".");
    delay(300);
  }
  Serial.println();
  Serial.print("Connected with IP: ");
  Serial.println(WiFi.localIP());
  Serial.println();

  /* Assign the api key (required) */
  config.api_key = API_KEY;

  /* Assign the user sign in credentials */
  auth.user.email = USER_EMAIL;
  auth.user.password = USER_PASSWORD;

  /* Assign the RTDB URL (required) */
  config.database_url = DATABASE_URL;

  // Comment or pass false value when WiFi reconnection will control by your code or third party library e.g. WiFiManager
  Firebase.reconnectNetwork(true);

  // Since v4.4.x, BearSSL engine was used, the SSL buffer need to be set.
  // Large data transmission may require larger RX buffer, otherwise connection issue or data read time out can be occurred.
  fbdo.setBSSLBufferSize(4096 /* Rx buffer size in bytes from 512 - 16384 */, 1024 /* Tx buffer size in bytes from 512 - 16384 */);

  // Limit the size of response payload to be collected in FirebaseData
  fbdo.setResponseSize(2048);

  Firebase.begin(&config, &auth);

  Firebase.setDoubleDigits(5);

  config.timeout.serverResponse = 10 * 1000;
}

void loop() {
  if ( ! rfid.PICC_IsNewCardPresent())
    return;
  if (rfid.PICC_ReadCardSerial()) {
    for (byte i = 0; i < 4; i++) {
      tag += rfid.uid.uidByte[i];
    }
    Serial.println(tag);
    rfid.PICC_HaltA();
    rfid.PCD_StopCrypto1();
  }

  num = tag.toDouble();
  tag = "";
   if (Firebase.ready() && (millis() - sendDataPrevMillis > 2000 || sendDataPrevMillis == 0))
  {
    sendDataPrevMillis = millis();

    Serial.printf("Gas Leak: %s\n", Firebase.RTDB.setDouble(&fbdo, F("/Responce"), num) ? "ok" : fbdo.errorReason().c_str());

  }
}
