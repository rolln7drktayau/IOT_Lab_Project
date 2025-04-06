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
}

void calibrateSensor()
{
    // Mesure de la valeur de base sans doigt
    long total = 0;
    for (int i = 0; i < 100; i++)
    {
        total += analogRead(PULSE_PIN);
        delay(10);
    }
    baselineValue = total / 100;
}

void loop()
{
    if (millis() - lastSampleTime >= SAMPLE_RATE)
    {
        lastSampleTime = millis();

        // Lecture du capteur
        int sensorValue = analogRead(PULSE_PIN);

        // Détection de présence du doigt
        int variation = abs(sensorValue - baselineValue);
        fingerDetected = variation > 50; // Seuil ajustable

        if (fingerDetected)
        {
            digitalWrite(LED_PIN, HIGH);

            // Suivi des pics et vallées
            if (sensorValue > peakValue)
                peakValue = sensorValue;
            if (sensorValue < valleyValue)
                valleyValue = sensorValue;

            // Détection de battement (pic significatif)
            if (sensorValue > baselineValue + (peakValue - baselineValue) * 0.7)
            {
                unsigned long currentTime = millis();

                // Calcul du BPM
                if (lastBeat > 0)
                {
                    unsigned long interval = currentTime - lastBeat;
                    BPM = 60000 / interval;
                }

                lastBeat = currentTime;

                // Réinitialisation pour le prochain cycle
                peakValue = sensorValue;
                valleyValue = 1023;
            }
        }
        else
        {
            digitalWrite(LED_PIN, LOW);

            // Réinitialisation quand pas de doigt
            BPM = 0;
            peakValue = 0;
            valleyValue = 1023;
            lastBeat = 0;
        }

        // Affichage des informations
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