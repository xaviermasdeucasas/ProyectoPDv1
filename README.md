# 📻 Radio Master ESP32

Reproductor de radio por internet (streaming HTTP/ICYcast) de alta fidelidad basado en el microcontrolador ESP32. Cuenta con un sistema de control dual que permite operar el dispositivo tanto desde pulsadores físicos como desde cualquier teléfono móvil a través de una aplicación web integrada.

Desarrollado como proyecto práctico de la asignatura **Procesadores Digitales** del grado en **Ingeniería de Sistemas Audiovisuales** (UPC ESEIAAT).

## ✨ Características Principales

* **Arquitectura FreeRTOS (Dual-Core):** El núcleo 0 está dedicado en exclusiva a la decodificación del flujo MP3 y la transmisión I2S, mientras que el núcleo 1 gestiona el servidor web, la pantalla OLED y la lectura de los botones físicos (evitando cortes en el audio por inanición).
* **Audio Digital I2S:** Salida de sonido limpia y potente mediante el DAC amplificador externo MAX98357A, esquivando las limitaciones del DAC interno del ESP32.
* **Control Anti-Clipping:** Limitador de ganancia digital por software implementado en la etapa de pre-amplificación para proteger el transductor y evitar la distorsión de la señal.
* **Interfaz Web AJAX (Mobile-First):** Servidor HTTP integrado que ofrece una app web de control. Utiliza peticiones asíncronas para una respuesta instantánea (< 1 ms) sin necesidad de recargar la página.
* **Sincronización Bidireccional:** El estado de la radio en la web (volumen y emisora actual) se actualiza en tiempo real mediante *HTTP Polling* si el usuario interactúa con los botones físicos de la placa.

## 🛠️ Hardware Utilizado

* **Microcontrolador:** ESP32 DevKit V1
* **Módulo de Audio:** DAC Amplificador I2S MAX98357A
* **Transductor:** Altavoz 3W 4Ω
* **Display:** Pantalla OLED SSD1306 (128x64) I2C
* **Control:** 3x Pulsadores táctiles (Configuración lógica `INPUT_PULLUP`)

### 🔌 Esquema de Conexiones (Pinout)

| Componente | Pin ESP32 | Función |
| :--- | :--- | :--- |
| **MAX98357A** | GPIO 26 | I2S BCLK |
| | GPIO 25 | I2S LRC |
| | GPIO 22 | I2S DIN |
| | GPIO 27 | Control SD (Encendido) |
| **OLED SSD1306**| GPIO 17 | I2C SDA |
| | GPIO 21 | I2C SCL |
| **Botones** | GPIO 32 | Volumen + |
| | GPIO 33 | Volumen - |
| | GPIO 23 | Cambio de Emisora |

## 💻 Dependencias y Librerías

El proyecto está configurado para **PlatformIO**. Se requieren las siguientes librerías (especificadas en el `platformio.ini`):

* `earlephilhower/ESP8266Audio` - Motor de decodificación MP3 y control I2S.
* `adafruit/Adafruit SSD1306` - Control del display OLED.
* `adafruit/Adafruit GFX Library` - Gráficos base para el OLED.

## 🚀 Instalación y Uso

1. Clona este repositorio:
```bash
   git clone [https://github.com/TuUsuario/Radio-Master-ESP32.git](https://github.com/TuUsuario/Radio-Master-ESP32.git)