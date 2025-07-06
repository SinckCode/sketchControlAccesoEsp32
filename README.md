# 🚪 Proyecto de Control de Acceso con ESP32

Este repositorio contiene una serie de **sketches** desarrollados en **Arduino IDE** para implementar un sistema de **control de acceso inteligente** basado en **ESP32**, que integra lectores RFID, control de barrera (pluma), sensores de ocupación y comunicación con una **API REST**.

---

## 📂 Estructura del Proyecto

A continuación se describen los principales directorios y versiones:

- **sketch_prueba1**  
  Primeros experimentos de conexión básica al ESP32 y validación del entorno de desarrollo.

- **sketch_RFID**  
  Implementación inicial de lectura de tarjetas RFID y visualización de UIDs por Serial Monitor.

- **sketch_Ver1MotorApi** a **sketch_Ver5MotorApi**  
  Versiones intermedias que integran gradualmente:
  - Control de servomotor.
  - Comunicación HTTP con una API REST.
  - Validación de credenciales.
  - Pruebas de latencia y consistencia de red.

- **sketch_Ver6MotorApi**  
  ✅ **Versión más completa y avanzada**, incluye:
  - Lectura de múltiples lectores RFID.
  - Validación con API REST.
  - Control del servomotor de la pluma.
  - **Detección de ocupación con sensor ultrasónico.**
  - Actualización del estado ocupado/libre en la API.
  - Encendido de LEDs según ocupación.
  - Lógica de cierre automático.

- **sketch_Ver7MotorApi**  
  ✅ **Versión simplificada**, pensada para uso básico:
  - Lectura RFID.
  - Validación con API REST.
  - Control de apertura/cierre de pluma.
  - **NO incluye sensor ultrasónico ni LEDs de ocupación.**

- **sketchfinal2**  
  Consolidación de funcionalidades principales de acceso y control de pluma.

---

## ⚙️ Requisitos del Sistema

### 📡 Hardware

- ESP32
- Lector RFID RC522 (1 o 2)
- Servomotor (por ejemplo, SG90)
- Sensor ultrasónico HC-SR04 (solo versión 6)
- LEDs indicadores (ocupado/libre)
- Fuente de alimentación adecuada (5V recomendada para servo y sensor, 3.3V para RFID)

### 🧰 Software

- Arduino IDE (versión recomendada 1.8.x o superior)
- Librerías necesarias:
  - `WiFi.h`
  - `HTTPClient.h`
  - `MFRC522.h`
  - `ESP32Servo.h`
- Drivers USB del ESP32 instalados en tu sistema operativo

---

## 🚦 Descripción del Funcionamiento

El sistema permite:

1. **Lectura de tarjetas RFID**
   - Identifica el UID de la tarjeta al aproximarla al lector.
   - Envía el UID mediante una solicitud HTTP POST a la API.

2. **Validación del acceso**
   - La API responde si el acceso es autorizado o denegado.

3. **Control de la barrera**
   - Si el acceso es válido:
     - El servomotor abre la pluma.
     - Se mantiene abierta durante un tiempo configurable.
     - Posteriormente se cierra automáticamente.

4. **Detección de ocupación** *(solo versión 6)*
   - El sensor ultrasónico detecta si el lugar está ocupado.
   - El estado se actualiza en la API mediante una solicitud PATCH.
   - LEDs indican estado:  
     🔴 Ocupado  
     🟢 Libre

5. **Control remoto**
   - La API puede abrir o cerrar la pluma remotamente si se autoriza desde el servidor.

---

## 🛠️ Pines y Conexiones

| Dispositivo          | Pin ESP32 | Descripción                  |
|----------------------|-----------|------------------------------|
| RFID 1 SS (SDA/CS)   | GPIO 5    | Primer lector RFID           |
| RFID 2 SS (SDA/CS)   | GPIO 21   | Segundo lector RFID          |
| RST (compartido)     | GPIO 22   | Reset de ambos lectores      |
| SCK                  | GPIO 18   | Reloj SPI                    |
| MISO                 | GPIO 19   | Datos desde lector           |
| MOSI                 | GPIO 23   | Datos hacia lector           |
| Servo control        | GPIO 17   | Movimiento de la pluma       |
| Sensor TRIG          | GPIO 26   | Disparo ultrasónico          |
| Sensor ECHO          | GPIO 25   | Eco ultrasónico              |
| LED Rojo             | GPIO 14   | Indicador de ocupado         |
| LED Verde            | GPIO 27   | Indicador de libre           |

---

## 🔑 Archivos Principales

| Archivo                        | Descripción                                                                                  |
|--------------------------------|----------------------------------------------------------------------------------------------|
| `sketch_Ver6MotorApi.ino`      | ✅ **Versión más completa**, con control de servo, RFID, sensor ultrasónico y actualización de LEDs. Recomendado para producción si necesitas todas las funcionalidades. |
| `sketch_Ver7MotorApi.ino`      | ✅ **Versión simplificada**, permite solo abrir/cerrar la pluma vía RFID o app, sin sensor de ocupación ni LEDs. Ideal para pruebas básicas o control de acceso sencillo. |
| Archivos `.zip`                | Copias de respaldo de todas las iteraciones anteriores.                                     |

---

## 🚀 Cómo Usar

1. **Clonar este repositorio**
   ```bash
   git clone https://github.com/tu-usuario/tu-repositorio.git
2. **Abrir el sketch deseado en Arduino IDE**
3. **Configurar tu red WiFi y la URL de la API**
   ```bash
    const char* ssid = "TU_RED_WIFI";
    const char* password = "TU_PASSWORD";
    const String BASE_URL = "http://tu-servidor/api";
4. **Seleccionar la placa ESP32 en Arduino IDE**
5. **Compilar y subir el sketch**
6. **Abrir el Monitor Serial para verificar el funcionamiento**

📝 Notas Importantes
Fuente de alimentación del servo: se recomienda alimentarlo con una fuente externa de 5V para evitar reinicios del ESP32.

Red WiFi estable: el sistema depende de conectividad continua.

API: asegúrate de que tu API REST esté corriendo y accesible desde la red donde se conectará el ESP32.

RFID: utiliza tarjetas Mifare compatibles con MFRC522.


Si quieres, puedes descargar la app y la api:

✅ **Repositorios:**  
API:
https://github.com/SinckCode/api-control-acceso
APP:
https://github.com/SinckCode/appControlAccess
