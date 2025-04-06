#include <Wire.h>

#define SDA_PIN 41 // J3 Pin 8 (GPIO41)
#define SCL_PIN 42 // J3 Pin 7 (GPIO42)

// Adresse I2C du TMG39931
#define TMG39931_I2C_ADDR 0x39

// Registres du TMG39931
#define TMG39931_ENABLE 0x80
#define TMG39931_ATIME 0x81
#define TMG39931_WTIME 0x83
#define TMG39931_AILTL 0x84
#define TMG39931_AILTH 0x85
#define TMG39931_AIHTL 0x86
#define TMG39931_AIHTH 0x87
#define TMG39931_PILT 0x89
#define TMG39931_PIHT 0x8B
#define TMG39931_PERS 0x8C
#define TMG39931_CONFIG1 0x8D
#define TMG39931_PPULSE 0x8E
#define TMG39931_CONTROL 0x8F
#define TMG39931_CONFIG2 0x90
#define TMG39931_ID 0x92
#define TMG39931_STATUS 0x93
#define TMG39931_CDATAL 0x94
#define TMG39931_CDATAH 0x95
#define TMG39931_RDATAL 0x96
#define TMG39931_RDATAH 0x97
#define TMG39931_GDATAL 0x98
#define TMG39931_GDATAH 0x99
#define TMG39931_BDATAL 0x9A
#define TMG39931_BDATAH 0x9B
#define TMG39931_PDATA 0x9C

bool pauseReading = false;

// Fonction pour écrire dans un registre
void writeRegister(byte reg, byte value)
{
  Wire.beginTransmission(TMG39931_I2C_ADDR);
  Wire.write(reg);
  Wire.write(value);
  Wire.endTransmission();
}

// Fonction pour lire un registre
byte readRegister(byte reg)
{
  Wire.beginTransmission(TMG39931_I2C_ADDR);
  Wire.write(reg);
  Wire.endTransmission();

  Wire.requestFrom(TMG39931_I2C_ADDR, 1);
  return Wire.read();
}

// Fonction pour initialiser le capteur
bool initializeTMG39931()
{
  // Vérifier l'ID du capteur
  byte id = readRegister(TMG39931_ID);
  Serial.print("ID du capteur: 0x");
  Serial.println(id, HEX);

  // Selon la documentation, l'ID devrait être 0xAB
  if (id != 0xAB)
  {
    Serial.println("ATTENTION: ID du capteur incorrect, mais on continue...");
  }

  // Désactiver le capteur pendant la configuration
  writeRegister(TMG39931_ENABLE, 0x00);
  delay(10);

  // Configurer le temps d'intégration ADC
  writeRegister(TMG39931_ATIME, 0xFF); // 2.7ms
  delay(10);

  // Configurer la persistance
  writeRegister(TMG39931_PERS, 0x00); // Chaque cycle génère une interruption
  delay(10);

  // Configurer le gain
  writeRegister(TMG39931_CONTROL, 0x00); // Gain 1x
  delay(10);

  // Activer le capteur (Power ON + capteur de couleur + capteur de proximité)
  writeRegister(TMG39931_ENABLE, 0x0F); // 0x0F = PON + AEN + PEN
  delay(10);

  return true;
}

void setup()
{
  Serial.begin(115200);
  while (!Serial)
    delay(10); // Attendre que le port série soit prêt

  Serial.println("\nDémarrage du programme TMG39931...");
  Serial.println("Initialisation I2C...");

  Wire.begin(SDA_PIN, SCL_PIN);
  Wire.setClock(100000); // Fréquence réduite pour plus de stabilité

  // Vérifier la présence du capteur sur le bus I2C
  Wire.beginTransmission(TMG39931_I2C_ADDR);
  byte error = Wire.endTransmission();

  if (error == 0)
  {
    Serial.println("Capteur TMG39931 détecté à l'adresse 0x39!");

    if (initializeTMG39931())
    {
      Serial.println("Capteur TMG39931 initialisé avec succès!");
    }
    else
    {
      Serial.println("Échec de l'initialisation du capteur!");
    }
  }
  else
  {
    Serial.println("Capteur TMG39931 non détecté. Vérifiez le câblage.");
    while (1)
    {
      delay(1000);
      Serial.println("Appuyez sur 'r' pour redémarrer...");
      if (Serial.available() && Serial.read() == 'r')
      {
        ESP.restart();
      }
    }
  }

  Serial.println("\nSystème prêt. Commandes disponibles:");
  Serial.println("p - Pause/Reprise des mesures");
  Serial.println("n - Prendre une nouvelle mesure (en mode pause)");
  Serial.println("r - Redémarrer l'ESP");
  Serial.println("Première mesure dans 3 secondes...");
  delay(3000);
}

// Fonction pour lire les données de couleur et de proximité
void readSensorData()
{
  // Lire la valeur de proximité
  uint8_t proximity = readRegister(TMG39931_PDATA);

  // Lire les valeurs de couleur
  uint16_t clear = readRegister(TMG39931_CDATAL) | (readRegister(TMG39931_CDATAH) << 8);
  uint16_t red = readRegister(TMG39931_RDATAL) | (readRegister(TMG39931_RDATAH) << 8);
  uint16_t green = readRegister(TMG39931_GDATAL) | (readRegister(TMG39931_GDATAH) << 8);
  uint16_t blue = readRegister(TMG39931_BDATAL) | (readRegister(TMG39931_BDATAH) << 8);

  // Afficher les données
  Serial.println("\n------ Nouvelles mesures ------");
  Serial.print("Proximité: ");
  Serial.println(proximity);

  Serial.println("Valeurs de couleur:");
  Serial.print("Rouge: ");
  Serial.println(red);
  Serial.print("Vert: ");
  Serial.println(green);
  Serial.print("Bleu: ");
  Serial.println(blue);
  Serial.print("Clear: ");
  Serial.println(clear);

  // Calculer le flux lumineux (approximation simple)
  float luminousFlux = 0.299 * red + 0.587 * green + 0.114 * blue;
  Serial.print("Flux lumineux (approximation): ");
  Serial.println(luminousFlux);

  // Triangulation chromatique confocale (approximation simple)
  // Normaliser les valeurs RGB
  float total = red + green + blue;
  if (total > 0)
  { // Éviter la division par zéro
    float rNorm = red / total;
    float gNorm = green / total;
    float bNorm = blue / total;

    Serial.println("Triangulation chromatique confocale:");
    Serial.print("Coordonnées chromatiques: (");
    Serial.print(rNorm, 3);
    Serial.print(", ");
    Serial.print(gNorm, 3);
    Serial.print(", ");
    Serial.print(bNorm, 3);
    Serial.println(")");

    // Calculer la température de couleur approximative (formule simplifiée)
    float X = 0.4124 * rNorm + 0.3576 * gNorm + 0.1805 * bNorm;
    float Y = 0.2126 * rNorm + 0.7152 * gNorm + 0.0722 * bNorm;
    float Z = 0.0193 * rNorm + 0.1192 * gNorm + 0.9505 * bNorm;

    float x = X / (X + Y + Z);
    float y = Y / (X + Y + Z);

    Serial.print("Coordonnées CIE: x=");
    Serial.print(x, 3);
    Serial.print(", y=");
    Serial.println(y, 3);
  }
  else
  {
    Serial.println("Intensité lumineuse trop faible pour calculer les coordonnées chromatiques");
  }
}

void loop()
{
  // Vérifier si une commande a été reçue
  if (Serial.available())
  {
    char cmd = Serial.read();
    if (cmd == 'r')
    {
      Serial.println("Redémarrage du système...");
      delay(100);
      ESP.restart();
    }
    else if (cmd == 'p')
    {
      pauseReading = !pauseReading;
      if (pauseReading)
      {
        Serial.println("\n*** PAUSE DES MESURES ***");
        Serial.println("Appuyez sur 'n' pour une mesure unique ou 'p' pour reprendre");
      }
      else
      {
        Serial.println("\n*** REPRISE DES MESURES ***");
      }
    }
    else if (cmd == 'n' && pauseReading)
    {
      readSensorData();
    }
  }

  // Si le système n'est pas en pause, prendre des mesures régulièrement
  if (!pauseReading)
  {
    readSensorData();

    Serial.println("\nProchaine mesure dans 10 secondes...");
    Serial.println("(Appuyez sur 'p' pour mettre en pause)");

    delay(10000); // Attendre 10 secondes entre les mesures
  }
  else
  {
    delay(100); // Vérifier les commandes plus fréquemment en mode pause
  }
}