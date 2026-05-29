// library lcd
#include <Wire.h>
#include <LiquidCrystal_I2C.h>

// library reader
#include <SPI.h>
#include <MFRC522.h>

// library wifi
#include <ESP8266WiFi.h>

// library http
#include <ESP8266HTTPClient.h>

// library https
#include <WiFiClientSecure.h>

// library doc
#include <ArduinoJson.h>

// alamat LCD biasanya 0x27 atau 0x3F
LiquidCrystal_I2C lcd(0x27, 16, 2);

// RFID
#define SS_PIN D4
#define RST_PIN D3
#define BUZZER D8

MFRC522 rfid(SS_PIN, RST_PIN);

// wifi
const char* ssid = "Yusuf habib";
const char* password = "ssssssss";

// api
// contoh API (ganti sesuai server kamu)
const char* serverUrl = "https://crud-peserta.yusuf-habib.blog/api/rfid";

void setup() {
  Serial.begin(115200);

  pinMode(BUZZER, OUTPUT);

  lcd.init();        // inisialisasi LCD
  lcd.backlight();

  lcd.setCursor(0,0);
  lcd.print("Menghubungkan");
  lcd.setCursor(0,1);
  lcd.print("WiFi");

  beep(1, 100);

  WiFi.begin(ssid, password);

  Serial.print("Connecting to ");
  Serial.print(ssid); Serial.println(" ...");  
  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
    lcd.print(".");
  }

  // int counter = 0;
  // while (WiFi.status() != WL_CONNECTED) {
  //   delay(500);
  //   Serial.print(".");
    
  //   lcd.setCursor(counter, 1);
  //   lcd.print(".");

  //   counter++;
  //   if (counter > 15) {
  //     counter = 0;
  //     lcd.setCursor(0,1);
  //     lcd.print("                "); // clear baris
  //   }
  // }

  // kalau berhasil konek
  lcd.clear();
  lcd.setCursor(0,0);
  lcd.print("WiFi Terhubung");
  lcd.setCursor(0,1);
  lcd.print(WiFi.localIP());

  Serial.println("\nWiFi Connected");
  Serial.println(WiFi.localIP());

  beep(2, 500);

  // RFID
  SPI.begin();
  rfid.PCD_Init();

}

// void loop() {

//   if (WiFi.status() == WL_CONNECTED) {


//     // http.begin(serverUrl);
//     // HTTPClient http;
//     // WiFiClient client;
//     WiFiClientSecure client;
//     client.setInsecure();
//     HTTPClient https;

//     https.begin(client, serverUrl);

//     https.addHeader("Content-Type", "application/json");

//     String jsonData = "{\"uid\":\"A1:B2:C3:D4\",\"status\":\"hadir\"}";

//     int httpResponseCode = https.POST(jsonData);

//     Serial.print("Response code: ");
//     Serial.println(httpResponseCode);

//     String response = https.getString();
//     // Serial.println(response);

//     StaticJsonDocument<200> doc;
//     deserializeJson(doc, response);

//     String msg = doc["message"];

//     lcd.clear();
//     lcd.setCursor(0,0);
//     lcd.print("Response code: ");
//     lcd.setCursor(0,1);
//     lcd.print(msg);

//     https.end();
//   }

//   delay(10000); // kirim tiap 10 detik (contoh)
// }

// void loop() {
//   // cek kartu baru
//   if (!rfid.PICC_IsNewCardPresent()) return;
//   if (!rfid.PICC_ReadCardSerial()) return;

//   // ambil UID
//   String uid = "";

//   for (byte i = 0; i < rfid.uid.size; i++) {
//     uid += String(rfid.uid.uidByte[i], HEX);
//     if (i < rfid.uid.size - 1) uid += ":";
//   }

//   uid.toUpperCase();

//   Serial.println("UID: " + uid);

//   // tampilkan ke LCD
//   lcd.clear();
//   lcd.setCursor(0,0);
//   lcd.print("UID:");
//   lcd.setCursor(0,1);
//   lcd.print(uid);

//   delay(2000);

//   lcd.clear();
//   lcd.setCursor(0,0);
//   lcd.print("Tempel Kartu");

//   beep(2, 100);

//   rfid.PICC_HaltA();
// }

void loop(){
  // tampilkan ke LCD
  lcd.clear();
  lcd.setCursor(0,0);
  lcd.print("Mulai Scan");
  lcd.setCursor(0,1);
  lcd.print("Tempelkan Kartu");

  if (!rfid.PICC_IsNewCardPresent()) return;
  if (!rfid.PICC_ReadCardSerial()) return;

  // ambil UID
  String uid = "";

  for (byte i = 0; i < rfid.uid.size; i++) {
    if (rfid.uid.uidByte[i] < 0x10) uid += "0"; // biar 2 digit
    uid += String(rfid.uid.uidByte[i], HEX);

    if (i < rfid.uid.size - 1) uid += ":";
  }

  uid.toUpperCase();

  // tampilkan ke LCD
  lcd.clear();
  lcd.setCursor(0, 0);
  lcd.print("UID:");
  lcd.setCursor(0, 1);
  lcd.print(uid);

  beep(2, 100);

 if (WiFi.status() == WL_CONNECTED) {

    WiFiClientSecure client;
    client.setInsecure();

    HTTPClient https;
    https.begin(client, serverUrl);
    https.addHeader("Content-Type", "application/json");

    // ===== JSON REQUEST =====
    StaticJsonDocument<200> docReq;
    docReq["uid"] = uid;
    docReq["gate"] = 1;

    String jsonData;
    serializeJson(docReq, jsonData);

    int httpResponseCode = https.POST(jsonData);

    // Serial.print("Response code: ");
    // Serial.println(httpResponseCode);

    // ===== RESPONSE =====
    String response = https.getString();

    StaticJsonDocument<200> docRes;
    DeserializationError error = deserializeJson(docRes, response);

    if (!error) {
        String status = docRes["status"].as<String>();
        String resuid = docRes["uid"].as<String>();

        lcd.clear();
        lcd.setCursor(0,0);
        lcd.print("Status: ");
        lcd.print(status);

        lcd.setCursor(0,1);
        lcd.print("UID: ");
        lcd.print(resuid);
    } else {
        Serial.println("JSON parse error");
    }

    https.end();
}
  // feedback
  // beep(2, 100);

  rfid.PICC_HaltA();

  delay(2000);
}

void beep(int times, int delayTime) {
  for (int i = 0; i < times; i++) {
    digitalWrite(BUZZER, HIGH);
    delay(delayTime);
    digitalWrite(BUZZER, LOW);
    delay(delayTime);
  }
}