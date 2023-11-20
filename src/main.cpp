#include <Arduino.h>
#include <WiFi.h>
#include <HTTPClient.h>
#include <ArduinoJson.h>
#include <MFRC522.h>
#include <SPI.h>

#define SS_PIN 5
#define RST_PIN 0

/*#define WIFI_SSID "ELTE"
#define WIFI_PASSWORD "Toma123"*/

#define WIFI_SSID "Rozsak"
#define WIFI_PASSWORD "1064rozsa"

const char* serverName = "https://192.168.0.139:44378/api/MagneticCard";

//const char* serverName = "https://recon20230521195539.azurewebsites.net/api/MagneticCard";

/// init Variable for RFID
byte nuidPICC[4] = {0, 0, 0, 0};
MFRC522::MIFARE_Key key;
MFRC522 rfid = MFRC522(SS_PIN, RST_PIN);

String apiKey = "TEST";

/// resposnse string init 
String response = "";
//JSON document
DynamicJsonDocument doc(2048);

void setup() {
  // Initialize the serial communication
  Serial.begin(921600);
  // Trying to connect to Wifi
  WiFi.mode(WIFI_STA);
  WiFi.begin(WIFI_SSID, WIFI_PASSWORD);
  Serial.println("Connecting ");

  // Wait for connection
  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }
  //Initialize the SPI bus
  SPI.begin();
  // Initialize the RFID reader
  rfid.PCD_Init();

  Serial.print("WiFi connected with IP: ");
  Serial.println(WiFi.localIP());
  
  Serial.print(F("Reader :"));
  rfid.PCD_DumpVersionToSerial();
}

void callApi(String cardID){
    /// Setup HttpClient
    HTTPClient http;
    http.addHeader("Content-Type", "application/x-www-form-urlencoded");
    /// set the URL
    String postData = "apiKey="+apiKey+"&cardId="+cardID;
    Serial.println(postData);
    String tmp = "?";
    String url = serverName+tmp+postData;
    // begin http connection
    http.begin(url);
   
    Serial.println(url);
    /// Send post request
    int httpResponseCode = http.POST(postData);
     if(httpResponseCode > 0 ){
        // Get response body
        String response = http.getString();  
  
        Serial.println(httpResponseCode);   
        Serial.println(response);     
    }else{
      Serial.print("Error on sending POST: ");
      Serial.println(httpResponseCode);
    }
    //End http connection
    http.end();
}

void ConvertByteToDec(byte *buffer, byte bufferSize) {
  /// Init Buffer string array 
  String val[bufferSize];
  for (byte i = 0; i < bufferSize; i++) {
    val[i] = buffer[i]; 
   
  }
  // Check if Wifi is connected 
  if(WiFi.status()== WL_CONNECTED){
    String cardID = "";
    for(int i = 0 ; i < bufferSize ; i ++ ){
      cardID+=val[i];
    }
    //Call api 
    callApi(cardID);
    
  }

}

void readRFID(void ) { 
  

  for (byte i = 0; i < 6; i++) {
    key.keyByte[i] = 0xFF;
  }

  // Check if new card 
  if ( ! rfid.PICC_IsNewCardPresent())
    return;

  // Read Card RFID SerialNumber
  if (  !rfid.PICC_ReadCardSerial())
    return;

  /// store UID of the card
  for (byte i = 0; i < 4; i++) {
    nuidPICC[i] = rfid.uid.uidByte[i];
  }

 
  ConvertByteToDec(rfid.uid.uidByte, rfid.uid.size);
  Serial.println();

 
  rfid.PICC_HaltA();

  // Stop encryption
  rfid.PCD_StopCrypto1();

}


void loop() {
  /// everytime call readRFID()
  readRFID();
}

