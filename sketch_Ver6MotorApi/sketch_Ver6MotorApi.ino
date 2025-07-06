#include <WiFi.h>
#include <HTTPClient.h>
#include <ESP32Servo.h>
#include <SPI.h>
#include <MFRC522.h>

// WiFi
const char* ssid = "GOLEMS";
const char* password = "123456789";

// API Base y endpoints
const String BASE_URL = "https://c3d2-200-68-185-27.ngrok-free.app";
const String LOGIN_URL = BASE_URL + "/api/auth/rfid-login";
const String PLUMA_STATUS_URL = BASE_URL + "/api/pluma/status";
const String PARKING_STATUS_BASE = BASE_URL + "/api/parking/status/";

// Pines RFID
#define SS_1 5
#define SS_2 21
#define RST_PIN 22
MFRC522 rfid1(SS_1, RST_PIN);
MFRC522 rfid2(SS_2, RST_PIN);

// Servo
#define SERVO_PIN 17
Servo plumaServo;

// Sensor ultrasónico
#define TRIG_PIN 26
#define ECHO_PIN 25

// LEDs
#define LED_ROJO 14
#define LED_VERDE 27

// Lugar asignado
#define SPOT_ID 1

// Estado pluma
bool plumaAbierta = false;
bool abiertaPorRFID = false;
unsigned long tiempoApertura = 0;
const unsigned long DURACION_APERTURA = 5000;

// Estado del espacio
bool estadoOcupadoActual = false;

void setup() {
  Serial.begin(115200);

  WiFi.begin(ssid, password);
  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }
  Serial.println("\n✅ WiFi conectado");

  // RFID
  SPI.begin(18, 19, 23); // VSPI
  rfid1.PCD_Init();
  rfid2.PCD_Init();

  // Servo
  plumaServo.attach(SERVO_PIN);
  plumaServo.write(0);

  // Sensor y LEDs
  pinMode(TRIG_PIN, OUTPUT);
  pinMode(ECHO_PIN, INPUT);
  pinMode(LED_ROJO, OUTPUT);
  pinMode(LED_VERDE, OUTPUT);
}

void loop() {
  if (WiFi.status() == WL_CONNECTED) {

    // Lector RFID 1
    if (!plumaAbierta && rfid1.PICC_IsNewCardPresent() && rfid1.PICC_ReadCardSerial()) {
      String uid = getUID(rfid1);
      Serial.println("📘 Lector 1 UID: " + uid);
      rfid1.PICC_HaltA();
      if (validarUID(uid)) abrirPluma(true);
      delay(1500);
    }

    // Lector RFID 2
    if (!plumaAbierta && rfid2.PICC_IsNewCardPresent() && rfid2.PICC_ReadCardSerial()) {
      String uid = getUID(rfid2);
      Serial.println("📗 Lector 2 UID: " + uid);
      rfid2.PICC_HaltA();
      if (validarUID(uid)) abrirPluma(true);
      delay(1500);
    }

    // Estado desde app
    String estado = obtenerEstadoPluma();
    if (estado == "open" && !plumaAbierta) {
      abrirPluma(false);
    } else if (estado == "closed" && plumaAbierta && !abiertaPorRFID) {
      cerrarPluma();
    }

    // Cierre automático
    if (plumaAbierta && abiertaPorRFID && (millis() - tiempoApertura > DURACION_APERTURA)) {
      Serial.println("⏱️ Cierre automático por tiempo");
      cerrarPluma();
    }

  } else {
    Serial.println("📡 WiFi desconectado");
  }

  // Sensor ultrasónico y LEDs
  float distancia = leerDistanciaCM();
  Serial.print("📏 Distancia: ");
  Serial.print(distancia);
  Serial.println(" cm");

  bool ocupado = (distancia > 2 && distancia <= 15);

  digitalWrite(LED_ROJO, ocupado ? HIGH : LOW);
  digitalWrite(LED_VERDE, ocupado ? LOW : HIGH);

  if (ocupado != estadoOcupadoActual) {
    if (ocupado) {
      Serial.println("🚗 Lugar ocupado (detectado por sensor)");
    } else {
      Serial.println("🅿️ Lugar libre");
    }

    actualizarEstadoEspacio(ocupado);
    estadoOcupadoActual = ocupado;
  }

  delay(1000);
}

// FUNCIONES

float leerDistanciaCM() {
  digitalWrite(TRIG_PIN, LOW);
  delayMicroseconds(2);
  digitalWrite(TRIG_PIN, HIGH);
  delayMicroseconds(10);
  digitalWrite(TRIG_PIN, LOW);

  long duracion = pulseIn(ECHO_PIN, HIGH, 30000);
  return duracion * 0.034 / 2;
}

void actualizarEstadoEspacio(bool ocupado) {
  HTTPClient http;
  String url = PARKING_STATUS_BASE + String(SPOT_ID);
  http.begin(url);
  http.addHeader("Content-Type", "application/json");

  String body = "{\"is_occupied\":";
  body += ocupado ? "true" : "false";
  body += ",\"is_reserved\":false}";

  int httpCode = http.PATCH(body);
  if (httpCode > 0) {
    Serial.print("📡 Estado enviado: ");
    Serial.println(body);
    String response = http.getString();
    Serial.println("🔁 Respuesta: " + response);
  } else {
    Serial.println("❌ Error al enviar estado: " + String(httpCode));
  }

  http.end();
}

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

String obtenerEstadoPluma() {
  HTTPClient http;
  http.begin(PLUMA_STATUS_URL);
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

void abrirPluma(bool porRFID) {
  plumaServo.write(90);
  plumaAbierta = true;
  abiertaPorRFID = porRFID;
  tiempoApertura = millis();
  Serial.println("🚦 Pluma abierta");
}

void cerrarPluma() {
  plumaServo.write(0);
  plumaAbierta = false;
  abiertaPorRFID = false;
  Serial.println("🚧 Pluma cerrada");
}
