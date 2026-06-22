#include <Arduino.h>
#include <WiFi.h>
#include <Wire.h>
#include <Adafruit_GFX.h>
#include <Adafruit_SSD1306.h>

// Librerías de audio oficiales
#include "AudioFileSourceICYStream.h" // Volvemos a ICYStream porque estas emisoras son nativas
#include "AudioFileSourceBuffer.h"
#include "AudioGeneratorMP3.h"
#include "AudioOutputI2S.h"

// Credenciales de vuestra red Wi-Fi de Movistar
const char* ssid     = "Nautilus";
const char* password = "20000Leguas";

// Asignación de pines de vuestro montaje
#define I2S_BCLK 26
#define I2S_LRC  25
#define I2S_DOUT 22
#define AMP_SD   27  // Control de encendido del amplificador

#define BTN_VOL_UP   32
#define BTN_VOL_DOWN 33
#define BTN_STATION  23 

#define OLED_SDA 17
#define OLED_SCL 21

Adafruit_SSD1306 display(128, 64, &Wire, -1);

// Punteros del decodificador de audio
AudioGeneratorMP3 *mp3 = NULL;
AudioFileSourceICYStream *file = NULL;
AudioFileSourceBuffer *buff = NULL;
AudioOutputI2S *out = NULL;

int volume = 7; // Volumen inicial al 7
int currentStation = 0;
unsigned long ultimoTiempoBoton = 0;

// Búfer de memoria preasignado (16KB) para máxima estabilidad
const int preallocateBufferSize = 16 * 1024; 
void *preallocateBuffer = NULL;

// ====================================================================
// NUEVAS EMISORAS: Servidores Icecast puros (Sin bloqueos CDN)
// ====================================================================
struct RadioStation {
    const char* url;
    const char* displayName;
};

RadioStation stationList[] = {
    {"http://stream.radioparadise.com/mp3-128", "Radio Paradise"}, // Rock/Indie alternativo
    {"http://ice1.somafm.com/groovesalad-128-mp3", "Groove Salad"}, // Chillout y electrónica suave
    {"http://strm112.1.fm/top40_mobile_mp3", "1.FM Top 40"},       // Éxitos Pop actuales
    {"http://strm112.1.fm/dance_mobile_mp3", "1.FM Dance"},        // Música Dance y Club
    {"http://ice1.somafm.com/defcon-128-mp3", "SomaFM DefCon"}     // Electrónica experimental
};
const int totalStations = 5;

void actualizarPantalla() {
    display.clearDisplay();
    display.setTextColor(SSD1306_WHITE);
    
    display.setCursor(0,0);
    display.setTextSize(1);
    display.println("RADIO MASTER OK");
    display.println("---------------------");
    
    display.setTextSize(2);
    display.setCursor(0,20);
    display.print("VOL: "); display.println(volume);
    
    display.setTextSize(1);
    display.setCursor(0,48);
    display.println("---------------------");
    display.print(stationList[currentStation].displayName);
    display.display();
}

void conectarAEmisora() {
    // Apagado limpio para no saturar la memoria RAM al cambiar de emisora
    if (mp3 && mp3->isRunning()) {
        mp3->stop();
    }
    if (buff) { buff->close(); delete buff; buff = NULL; }
    if (file) { file->close(); delete file; file = NULL; }

    display.clearDisplay();
    display.setCursor(0,10);
    display.setTextSize(1);
    display.println("Sintonizando...");
    display.println(stationList[currentStation].displayName);
    display.display();

    Serial.printf("\nConectando a: %s\n", stationList[currentStation].displayName);

    // Inicializamos el flujo de audio con las emisoras seguras
    file = new AudioFileSourceICYStream(stationList[currentStation].url);
    buff = new AudioFileSourceBuffer(file, preallocateBuffer, preallocateBufferSize);
    
    if (mp3->begin(buff, out)) {
        Serial.println("¡Audio fluyendo! Decodificando MP3...");
    } else {
        Serial.println("Error de red. Reintentando...");
    }
}

void setup() {
    Serial.begin(115200);
    delay(500);

    // Activamos el amplificador desde el pin G27
    pinMode(AMP_SD, OUTPUT);
    digitalWrite(AMP_SD, HIGH); 
    Serial.println("Amplificador MAX98357A: ENCENDIDO");

    // Reservamos la memoria RAM estática
    preallocateBuffer = malloc(preallocateBufferSize);
    if (!preallocateBuffer) {
        Serial.println("Error: RAM insuficiente");
        while(1) delay(1000);
    }

    Wire.begin(OLED_SDA, OLED_SCL);
    if(!display.begin(SSD1306_SWITCHCAPVCC, 0x3C)) Serial.println("Error OLED");
    
    display.clearDisplay();
    display.setTextColor(SSD1306_WHITE);
    display.setCursor(0,10);
    display.println("Conectando Wi-Fi...");
    display.display();

    // Conexión Wi-Fi limpia y normal (Sin hacks de IP, estas emisoras no lo necesitan)
    WiFi.begin(ssid, password);
    while (WiFi.status() != WL_CONNECTED) {
        delay(500);
        Serial.print(".");
    }
    Serial.println("\n¡Wi-Fi Movistar Conectado!");
    Serial.print("IP: "); Serial.println(WiFi.localIP());

    pinMode(BTN_VOL_UP, INPUT_PULLUP);
    pinMode(BTN_VOL_DOWN, INPUT_PULLUP);
    pinMode(BTN_STATION, INPUT_PULLUP);

    // Configuración de la salida I2S
    out = new AudioOutputI2S();
    out->SetPinout(I2S_BCLK, I2S_LRC, I2S_DOUT);
    out->SetGain((float)volume / 21.0); 
    
    mp3 = new AudioGeneratorMP3();
    
    conectarAEmisora();
    actualizarPantalla();
}

void loop() {
    // Proceso continuo de lectura de MP3 y envío a I2S
    if (mp3 && mp3->isRunning()) {
        if (!mp3->loop()) {
            mp3->stop();
            Serial.println("El stream se ha cortado o reiniciado.");
        }
    }

    // Botones con tiempo de seguridad anti-rebotes
    if (millis() - ultimoTiempoBoton > 300) {
        
        if(digitalRead(BTN_VOL_UP) == LOW) {
            if(volume < 21) { 
                volume++; 
                out->SetGain((float)volume / 21.0);
                actualizarPantalla(); 
            }
            ultimoTiempoBoton = millis();
        }

        if(digitalRead(BTN_VOL_DOWN) == LOW) { 
            if(volume > 0) { 
                volume--; 
                out->SetGain((float)volume / 21.0);
                actualizarPantalla(); 
            }
            ultimoTiempoBoton = millis();
        }

        if(digitalRead(BTN_STATION) == LOW) {
            currentStation = (currentStation + 1) % totalStations;
            conectarAEmisora();
            actualizarPantalla();
            ultimoTiempoBoton = millis();
            
            // Pausa física para evitar doble salto
            while(digitalRead(BTN_STATION) == LOW) { delay(10); } 
        }
    }
}