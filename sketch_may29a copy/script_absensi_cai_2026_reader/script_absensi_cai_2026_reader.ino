// Script arduino reader
// desc : untuk mengirim data UID sebagai absensi peserta cai ke api
#include <Wire.h>
#include <LiquidCrystal_I2C.h>
#include <SPI.h>
#include <MFRC522.h>
#include <ESP8266WiFi.h>
#include <ESP8266HTTPClient.h>
#include <WiFiClientSecure.h>
#include <ArduinoJson.h>

// ================= LCD =================
LiquidCrystal_I2C lcd(0x27, 16, 2);

// ================= RFID =================
#define SS_PIN D4
#define RST_PIN D3
#define BUZZER  D8

MFRC522 rfid(SS_PIN, RST_PIN);

// ================= WIFI =================
const char* ssid = "Yusuf habib";
// const char* ssid = "zaydenoman";
const char* password = "ssssssss";
// const char* password = "ocamocim354@";

// ================= API =================
const char* serverUrl = "https://crud-peserta.yusuf-habib.blog/api/absensi";

// ================= STATE =================
String lastUID = "";
unsigned long lastScanTime = 0;
const unsigned long scanCooldown = 3000; // anti double scan 3 detik

// ================= SETUP =================
void setup() {
  Serial.begin(115200);

  pinMode(BUZZER, OUTPUT);

  lcd.init();
  lcd.backlight();

  showLCD("Connecting WiFi", "");

  WiFi.begin(ssid, password);

  while (WiFi.status() != WL_CONNECTED) {
    delay(300);
    Serial.print(".");
  }

  lcd.clear();
  showLCD("WiFi Connected", WiFi.localIP().toString());

  delay(1500);

  SPI.begin();
  rfid.PCD_Init();

  showLCD("Tempel Kartu", "Siap Scan");
}

// ================= LOOP =================
void loop() {

  // tampil idle (tanpa spam clear terus)
  if (!rfid.PICC_IsNewCardPresent()) return;
  if (!rfid.PICC_ReadCardSerial()) return;

  String uid = getUID();

  // anti scan ulang kartu yang sama
  if (uid == lastUID && millis() - lastScanTime < scanCooldown) {
    rfid.PICC_HaltA();
    return;
  }

  lastUID = uid;
  lastScanTime = millis();

  Serial.println("UID: " + uid);

  showLCD("UID:", uid);

  beep(1, 80);

  sendToServer(uid);

  rfid.PICC_HaltA();

  delay(1500);
  showLCD("Tempel Kartu", "Siap Scan");
}

// ================= GET UID =================
String getUID() {
  String uid = "";

  for (byte i = 0; i < rfid.uid.size; i++) {
    if (rfid.uid.uidByte[i] < 0x10) uid += "0";
    uid += String(rfid.uid.uidByte[i], HEX);

    if (i < rfid.uid.size - 1) uid += ":";
  }

  uid.toUpperCase();
  return uid;
}

// ================= SEND API =================
void sendToServer(String uid) {

  if (WiFi.status() != WL_CONNECTED) {
    showLCD("WiFi Lost", "");
    return;
  }

  WiFiClientSecure client;
  client.setInsecure();

  HTTPClient https;
  https.begin(client, serverUrl);
  https.addHeader("Content-Type", "application/json");

  StaticJsonDocument<200> doc;
  doc["uid"] = uid;
  doc["gate"] = 1;

  String payload;
  serializeJson(doc, payload);

  int code = https.POST(payload);

  String response = https.getString();

  Serial.print("HTTP: ");
  Serial.println(code);

  StaticJsonDocument<256> res;
  DeserializationError err = deserializeJson(res, response);

  if (!err) {
    String status = res["status"] | "unknown";
    // String resuid = res["uid"] | uid;
    String resmessage = res["message"];
    String resnama = res["nama"] | "unknown";

    showLCD(resnama, status);
  } else {
    Serial.println("JSON error");
    showLCD("Server Error", "");
  }

  https.end();
}

// ================= LCD HELPER =================
void showLCD(String line1, String line2) {
  lcd.clear();
  lcd.setCursor(0, 0);
  lcd.print(line1);
  lcd.setCursor(0, 1);
  lcd.print(line2);
}

// ================= BUZZER =================
void beep(int times, int d) {
  for (int i = 0; i < times; i++) {
    digitalWrite(BUZZER, HIGH);
    delay(d);
    digitalWrite(BUZZER, LOW);
    delay(d);
  }
}