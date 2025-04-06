#include <Wire.h>
#include "HT_SSD1306Wire.h"

// Configuration de l'écran OLED
static SSD1306Wire display(0x3c, 500000, SDA_OLED, SCL_OLED, GEOMETRY_128_64, RST_OLED);

// Configuration du capteur de pouls
#define PULSE_PIN 1 // GPIO1 sur J3 pin 12
#define LED_PIN 25  // LED intégrée Heltec

// Variables de détection
unsigned long lastSampleTime = 0;
const int SAMPLE_RATE = 10; // Échantillonnage rapide

// Variables pour la détection de pouls
int baselineValue = 0;
int peakValue = 0;
int valleyValue = 1023;
bool fingerDetected = false;
unsigned long lastPeakTime = 0;

// Variables de calcul du BPM
volatile int BPM = 0;
volatile unsigned long lastBeat = 0;

// Variables pour la courbe
int sensorValues[128];
int sensorIndex = 0;

// Variables pour le filtre moyenne mobile
const int WINDOW_SIZE = 5;
int sensorWindow[WINDOW_SIZE];
int windowIndex = 0;

void setup()
{
    Serial.begin(115200);
    pinMode(PULSE_PIN, INPUT);
    pinMode(LED_PIN, OUTPUT);

    // Calibration initiale
    calibrateSensor();

    Serial.println("=== Iduino Heartbeat Sensor ===");
    Serial.print("Valeur de baseline : ");
    Serial.println(baselineValue);
    Serial.println("Placez votre doigt et attendez...");

    // Initialisation de l'écran OLED
    pinMode(Vext, OUTPUT);
    digitalWrite(Vext, LOW);
    delay(100);
    display.init();
    display.clear();

    // Initialisation du tableau de valeurs
    for (int i = 0; i < 128; i++)
    {
        sensorValues[i] = baselineValue;
    }

    // Initialisation du filtre moyenne mobile
    for (int i = 0; i < WINDOW_SIZE; i++)
    {
        sensorWindow[i] = baselineValue;
    }
}

void calibrateSensor()
{
    long total = 0;
    for (int i = 0; i < 100; i++)
    {
        total += analogRead(PULSE_PIN);
        delay(10);
    }
    baselineValue = total / 100;
}

int movingAverage(int newValue)
{
    sensorWindow[windowIndex] = newValue;
    windowIndex = (windowIndex + 1) % WINDOW_SIZE;
    int sum = 0;
    for (int i = 0; i < WINDOW_SIZE; i++)
    {
        sum += sensorWindow[i];
    }
    return sum / WINDOW_SIZE;
}

void loop()
{
    if (millis() - lastSampleTime >= SAMPLE_RATE)
    {
        lastSampleTime = millis();

        int sensorValue = analogRead(PULSE_PIN);

        // Filtre moyenne mobile
        sensorValue = movingAverage(sensorValue);

        int variation = abs(sensorValue - baselineValue);
        fingerDetected = variation > 50;

        if (fingerDetected)
        {
            digitalWrite(LED_PIN, HIGH);

            if (sensorValue > peakValue)
                peakValue = sensorValue;
            if (sensorValue < valleyValue)
                valleyValue = sensorValue;

            // Seuil de détection de battement ajusté
            if (sensorValue > baselineValue + (peakValue - baselineValue) * 0.3)
            {
                unsigned long currentTime = millis();

                if (lastBeat > 0)
                {
                    unsigned long interval = currentTime - lastBeat;
                    if (interval > 300 && interval < 2000)
                    { // Validation de l'intervalle
                        BPM = 60000 / interval;
                        lastBeat = currentTime;
                    }
                }
                else
                {
                    lastBeat = currentTime;
                }
                peakValue = sensorValue;
                valleyValue = 1023;
            }
        }
        else
        {
            digitalWrite(LED_PIN, LOW);
            BPM = 0;
            peakValue = 0;
            valleyValue = 1023;
            lastBeat = 0;
        }

        // Ajout de la valeur à la courbe
        sensorValues[sensorIndex] = sensorValue;
        sensorIndex = (sensorIndex + 1) % 128;

        // Affichage sur l'écran OLED
        display.clear();
        display.setTextAlignment(TEXT_ALIGN_LEFT);
        display.setFont(ArialMT_Plain_10);
        display.drawString(0, 0, "Signal: " + String(sensorValue));
        display.drawString(0, 15, "BPM: " + String(BPM));
        display.drawString(0, 30, "Doigt: " + String(fingerDetected ? "PRÉSENT" : "ABSENT"));
        display.drawString(0, 45, "Variation: " + String(variation));

        // Dessiner la courbe
        for (int x = 0; x < 127; x++)
        {
            int y1 = map(sensorValues[x], baselineValue - 50, peakValue + 50, 63, 0);
            int y2 = map(sensorValues[x + 1], baselineValue - 50, peakValue + 50, 63, 0);
            display.drawLine(x, y1, x + 1, y2);
        }

        display.display();

        // Affichage sur le moniteur série (pour le débogage)
        Serial.print("Signal: ");
        Serial.print(sensorValue);
        Serial.print(" | BPM: ");
        Serial.print(BPM);
        Serial.print(" | Doigt: ");
        Serial.print(fingerDetected ? "PRÉSENT" : "ABSENT");
        Serial.print(" | Variation: ");
        Serial.println(variation);

        // Pour Serial Plotter
        Serial.print(sensorValue);
        Serial.print("\t");
        Serial.print(BPM);
        Serial.print("\t");
        Serial.println(variation);
    }
}