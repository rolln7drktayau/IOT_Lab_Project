#include <Wire.h>
#include <U8g2lib.h>
#include <RadioLib.h>
#include <SPI.h>

#define SDA_PIN 41
#define SCL_PIN 42
#define OLED_SDA 17
#define OLED_SCL 18
#define OLED_RST 21
#define LORA_SCK 9
#define LORA_MISO 11
#define LORA_MOSI 10
#define LORA_SS 8
#define LORA_RST 12
#define LORA_DIO0 14
#define LORA_BUSY 13
#define LORA_FREQUENCY 868.0

SX1262 radio = new Module(LORA_SS, LORA_DIO0, LORA_RST, LORA_BUSY);
U8G2_SSD1306_128X64_NONAME_F_SW_I2C u8g2(U8G2_R0, OLED_SCL, OLED_SDA, OLED_RST);

// Variables simulées pour données capteurs (à remplacer par vraies lectures)
uint8_t proximity = 10;
uint16_t clear = 100, red = 50, green = 70, blue = 90;

// Variables pour la confirmation de transmission
bool transmissionSuccess = false;
unsigned long lastTransmissionTime = 0;
int packetsSent = 0;
int packetsAcknowledged = 0;

void setup()
{
    Serial.begin(115200);

    // Init OLED
    u8g2.begin();
    u8g2.clearBuffer();
    u8g2.setFont(u8g2_font_6x10_tf);
    u8g2.drawStr(0, 10, "Init LoRa Sender...");
    u8g2.sendBuffer();

    // Init LoRa
    SPI.begin(LORA_SCK, LORA_MISO, LORA_MOSI, LORA_SS);
    if (radio.begin(LORA_FREQUENCY) != RADIOLIB_ERR_NONE)
    {
        Serial.println("LoRa init failed!");
        while (true)
            ;
    }

    // Configuration pour la réception d'ACK
    radio.setDio1Action(setFlag);

    Serial.println("LoRa Sender ready");

    // Affichage initial
    updateDisplay("LoRa ready", "", "", true);
}

// Flag pour indiquer qu'une transmission a été reçue
volatile bool receivedFlag = false;

// Fonction pour définir le drapeau lors de la réception
void setFlag(void)
{
    receivedFlag = true;
}

void loop()
{
    // Simuler lecture capteurs (à remplacer par vraies lectures)
    proximity = random(5, 50);
    clear = random(80, 150);
    red = random(40, 70);
    green = random(60, 90);
    blue = random(80, 120);

    // Préparer données JSON
    String jsonData = "{";
    jsonData += "\"proximity\":" + String(proximity) + ",";
    jsonData += "\"clear\":" + String(clear) + ",";
    jsonData += "\"red\":" + String(red) + ",";
    jsonData += "\"green\":" + String(green) + ",";
    jsonData += "\"blue\":" + String(blue);
    jsonData += "}";

    // Afficher préparation d'envoi
    updateDisplay("Sending...",
                  ("Prox: " + String(proximity)).c_str(),
                  ("R:" + String(red) + " G:" + String(green) + " B:" + String(blue)).c_str(),
                  false);

    // Envoi LoRa
    int state = radio.transmit(jsonData);
    packetsSent++;
    lastTransmissionTime = millis();

    if (state == RADIOLIB_ERR_NONE)
    {
        Serial.println("Data Successfully sent : " + jsonData);
        transmissionSuccess = true;

        // Attendre un ACK pendant 1 seconde
        unsigned long ackTimeout = millis();
        radio.startReceive();

        while ((millis() - ackTimeout) < 1000)
        {
            if (receivedFlag)
            {
                receivedFlag = false;

                // Lire la réponse
                String response = "";
                int responseState = radio.readData(response);

                if (responseState == RADIOLIB_ERR_NONE && response == "ACK")
                {
                    Serial.println("ACK received !");
                    packetsAcknowledged++;
                    break;
                }
            }
            delay(10);
        }

        // Mettre à jour l'affichage avec confirmation
        updateDisplay("Success !",
                      ("Prox: " + String(proximity)).c_str(),
                      ("R:" + String(red) + " G:" + String(green) + " B:" + String(blue)).c_str(),
                      true);
    }
    else
    {
        Serial.println("Error sending to LoRa : " + String(state));
        transmissionSuccess = false;

        // Mettre à jour l'affichage avec erreur
        updateDisplay("Sending Error !",
                      ("Err: " + String(state)).c_str(),
                      "",
                      true);
    }

    delay(5000);
}

// Fonction pour mettre à jour l'affichage OLED
void updateDisplay(const char *status, const char *line1, const char *line2, bool showStats)
{
    u8g2.clearBuffer();
    u8g2.setFont(u8g2_font_6x10_tf);

    // Ligne de statut en haut
    u8g2.drawStr(0, 10, status);

    // Données du capteur
    u8g2.drawStr(0, 25, line1);
    u8g2.drawStr(0, 35, line2);

    // Statistiques de transmission
    if (showStats)
    {
        String statsLine1 = "Pkts: " + String(packetsSent) + " ACK: " + String(packetsAcknowledged);
        String statsLine2 = "Tx Rate: " + String((packetsAcknowledged * 100) / (packetsSent > 0 ? packetsSent : 1)) + "%";

        u8g2.drawStr(0, 50, statsLine1.c_str());
        u8g2.drawStr(0, 60, statsLine2.c_str());
    }

    u8g2.sendBuffer();
}