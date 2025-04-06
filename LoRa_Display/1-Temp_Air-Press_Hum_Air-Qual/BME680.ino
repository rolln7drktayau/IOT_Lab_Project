#include <Wire.h>
#include <Adafruit_Sensor.h>
#include <Adafruit_BME680.h>
#include <LoRa.h>
#include <SSD1306Wire.h>

// Configuration de l'écran OLED
static SSD1306Wire display(0x3c, 500000, SDA_OLED, SCL_OLED, GEOMETRY_128_64, RST_OLED);

// Déclaration du capteur BME680
Adafruit_BME680 bme;

// Définition des broches LoRa
#define SS 5
#define RST 14
#define DIO0 2

void setup()
{
    // Initialisation de la communication série
    Serial.begin(115200);

    // Initialisation de l'écran OLED
    display.init();
    display.clear();

    // Initialisation du capteur BME680
    if (!bme.begin())
    {
        Serial.println("Erreur BME680");
        while (1)
            ;
    }

    // Initialisation du module LoRa
    LoRa.setPins(SS, RST, DIO0);
    if (!LoRa.begin(868E6))
    {
        Serial.println("Erreur LoRa");
        while (1)
            ;
    }

    Serial.println("Capteur et LoRa initialisés !");
}

void loop()
{
    // Demander la lecture des données
    if (bme.performReading())
    {
        float temperature = bme.temperature;
        float humidity = bme.humidity;
        float pressure = bme.pressure / 100.0;
        int air_quality = bme.gas_resistance;

        // Afficher les données sur l'écran OLED
        display.clear();
        display.setTextAlignment(TEXT_ALIGN_LEFT);
        display.setFont(ArialMT_Plain_10);

        display.drawString(0, 0, "Temp: " + String(temperature) + " C");
        display.drawString(0, 12, "Humidity: " + String(humidity) + " %");
        display.drawString(0, 24, "Pressure: " + String(pressure) + " hPa");
        display.drawString(0, 36, "Air Quality: " + String(air_quality));

        // Afficher les données sur le terminal série
        Serial.print("Temp: ");
        Serial.print(temperature);
        Serial.print(" C, Humidity: ");
        Serial.print(humidity);
        Serial.print(" %, Pressure: ");
        Serial.print(pressure);
        Serial.print(" hPa, Air Quality: ");
        Serial.println(air_quality);

        display.display();

        // Envoi des données via LoRa
        LoRa.beginPacket();
        LoRa.print("Temp: " + String(temperature));
        LoRa.print(" Humidity: " + String(humidity));
        LoRa.print(" Pressure: " + String(pressure));
        LoRa.print(" Air Quality: " + String(air_quality));
        LoRa.endPacket();

        // Attendre 5 secondes avant de prendre une nouvelle lecture
        delay(5000);
    }
    else
    {
        Serial.println("Erreur de lecture du capteur");
    }
}
