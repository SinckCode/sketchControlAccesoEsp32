#include <WiFi.h>
#include <HTTPClient.h>
#include <ESP32Servo.h>
#include <SPI.h>
#include <MFRC522.h>

// WiFi
const char* ssid = "Totalplay-8CAA";
const char* password = "8CAA922DxN7xJVBT";

// API
const String BASE_URL = "https://b91f-207-249-176-150.ngrok-free.app";
const String LOGIN_URL = BASE_URL + "/api/auth/rfid-login";
const String STATUS_URL = BASE_URL + "/api/pluma2/status"; // 👉 CAMBIADA A PUERTA 2

// Pines RFID
#define SS_1 5
#define SS_2 21
#define RST_PIN 22

MFRC522 rfid1(SS_1, RST_PIN);
MFRC522 rfid2(SS_2, RST_PIN);

// Servo
#define SERVO_PIN 17
Servo plumaServo;

// Estado
bool plumaAbierta = false;
bool abiertaPorRFID = false;
unsigned long tiempoApertura = 0;
const unsigned long DURACION_APERTURA = 5000;

// Declaraciones necesarias
void abrirPluma(bool porRFID = false);
void cerrarPluma();

void setup() {
  Serial.begin(115200);
  WiFi.begin(ssid, password);
  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }
  Serial.println("\n✅ WiFi conectado");

  SPI.begin(18, 19, 23); // VSPI
  rfid1.PCD_Init();
  rfid2.PCD_Init();

  plumaServo.attach(SERVO_PIN);
  plumaServo.write(0); // Pluma cerrada
}

void loop() {
  if (WiFi.status() == WL_CONNECTED) {

    // 1. Lector RFID 1
    if (!plumaAbierta && rfid1.PICC_IsNewCardPresent() && rfid1.PICC_ReadCardSerial()) {
      String uid = getUID(rfid1);
      Serial.println("📘 Lector 1 UID: " + uid);
      rfid1.PICC_HaltA();
      if (validarUID(uid)) abrirPluma(true);
      delay(1500);
    }

    // 2. Lector RFID 2
    if (!plumaAbierta && rfid2.PICC_IsNewCardPresent() && rfid2.PICC_ReadCardSerial()) {
      String uid = getUID(rfid2);
      Serial.println("📗 Lector 2 UID: " + uid);
      rfid2.PICC_HaltA();
      if (validarUID(uid)) abrirPluma(true);
      delay(1500);
    }

    // 3. Verificación manual desde app
    String estado = obtenerEstadoPluma();
    if (estado == "open" && !plumaAbierta) {
      abrirPluma(false);  // desde app
    } else if (estado == "closed" && plumaAbierta && !abiertaPorRFID) {
      cerrarPluma();      // manual desde app
    }

    // 4. Cierre automático si se abrió por RFID
    if (plumaAbierta && abiertaPorRFID && (millis() - tiempoApertura > DURACION_APERTURA)) {
      Serial.println("⏱️ Cierre automático por tiempo");
      cerrarPluma();
    }

  } else {
    Serial.println("📡 WiFi desconectado");
  }

  delay(1000);
}

// Función para leer UID
String getUID(MFRC522 &lector) {
  String uid = "";
  for (byte i = 0; i < lector.uid.size; i++) {
    if (lector.uid.uidByte[i] < 0x10) uid += "0";
    uid += String(lector.uid.uidByte[i], HEX);
  }
  uid.trim();
  uid.toUpperCase();
  return uid;
}

// Validación de UID en API
bool validarUID(String uid) {
  uid.trim();
  uid.toUpperCase();
  Serial.println("📤 Enviando UID para validación: " + uid);

  HTTPClient http;
  http.begin(LOGIN_URL);
  http.addHeader("Content-Type", "application/json");

  String body = "{\"rfid_uid\":\"" + uid + "\"}";
  Serial.println("📝 Cuerpo enviado: " + body);

  int httpCode = http.POST(body);

  if (httpCode == 200) {
    String payload = http.getString();
    Serial.println("✅ Respuesta API: " + payload);
    http.end();
    return true;
  }

  Serial.println("⛔ RFID denegado o error (" + String(httpCode) + ")");
  http.end();
  return false;
}

// Consultar estado actual de la pluma
String obtenerEstadoPluma() {
  HTTPClient http;
  http.begin(STATUS_URL); // 👉 USAMOS LA RUTA NUEVA
  int httpCode = http.GET();

  if (httpCode == 200) {
    String result = http.getString();
    Serial.println("📥 Estado pluma: " + result);
    http.end();
    if (result.indexOf("open") != -1) return "open";
    if (result.indexOf("closed") != -1) return "closed";
  }

  http.end();
  return "unknown";
}

// Abrir pluma
void abrirPluma(bool porRFID) {
  plumaServo.write(90);
  plumaAbierta = true;
  abiertaPorRFID = porRFID;
  tiempoApertura = millis();
  Serial.println("🚦 Pluma 2 abierta");
}

// Cerrar pluma
void cerrarPluma() {
  plumaServo.write(0);
  plumaAbierta = false;
  abiertaPorRFID = false;
  Serial.println("🚧 Pluma 2 cerrada");
}
