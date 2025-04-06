#include <Wire.h>

#define SDA_PIN 41 // GPIO41 -> SDA (J3 pin 8)
#define SCL_PIN 42 // GPIO42 -> SCL (J3 pin 7)

#define BME680_I2C_ADDR 0x76 // Adresse I2C du BME680

// Registres utiles
#define BME680_CHIP_ID 0xD0
#define BME680_RESET 0xE0
#define BME680_CTRL_HUM 0x72
#define BME680_CTRL_MEAS 0x74
#define BME680_CONFIG 0x75
#define BME680_CTRL_GAS_1 0x71
#define BME680_STATUS 0x73

#define BME680_TEMP_MSB 0x22
#define BME680_PRESS_MSB 0x1F
#define BME680_HUM_MSB 0x25
#define BME680_GAS_RES_MSB 0x2A

void writeRegister(uint8_t reg, uint8_t val)
{
    Wire.beginTransmission(BME680_I2C_ADDR);
    Wire.write(reg);
    Wire.write(val);
    Wire.endTransmission();
}

uint8_t readRegister(uint8_t reg)
{
    Wire.beginTransmission(BME680_I2C_ADDR);
    Wire.write(reg);
    Wire.endTransmission();
    Wire.requestFrom(BME680_I2C_ADDR, 1);
    return Wire.read();
}

void readDataBlock(uint8_t reg, uint8_t *buffer, uint8_t length)
{
    Wire.beginTransmission(BME680_I2C_ADDR);
    Wire.write(reg);
    Wire.endTransmission();
    Wire.requestFrom(BME680_I2C_ADDR, length);
    for (uint8_t i = 0; i < length; i++)
    {
        buffer[i] = Wire.read();
    }
}

void setup()
{
    Serial.begin(115200);
    Wire.begin(SDA_PIN, SCL_PIN);
    Wire.setClock(100000);

    Serial.println("\nInitialisation du capteur BME680...");

    uint8_t chipID = readRegister(BME680_CHIP_ID);
    if (chipID != 0x61)
    {
        Serial.print("Erreur : ID inattendu: 0x");
        Serial.println(chipID, HEX);
        while (1)
            ;
    }

    // Reset capteur
    writeRegister(BME680_RESET, 0xB6);
    delay(100);

    // Configurations simples
    writeRegister(BME680_CTRL_HUM, 0x01);   // oversampling humidité x1
    writeRegister(BME680_CTRL_MEAS, 0x25);  // Temp x1, Press x1, mode Forced
    writeRegister(BME680_CONFIG, 0x00);     // Pas de filtre
    writeRegister(BME680_CTRL_GAS_1, 0x10); // Activer capteur gaz

    Serial.println("BME680 prêt.");
}

void loop()
{
    // Relancer une mesure
    writeRegister(BME680_CTRL_MEAS, 0x25);
    delay(200); // Attente acquisition

    uint8_t data[15];
    readDataBlock(0x1F, data, 15);

    int32_t adc_pres = ((uint32_t)data[0] << 12) | ((uint32_t)data[1] << 4) | (data[2] >> 4);
    int32_t adc_temp = ((uint32_t)data[3] << 12) | ((uint32_t)data[4] << 4) | (data[5] >> 4);
    int32_t adc_hum = ((uint16_t)data[6] << 8) | data[7];
    int16_t gas_res = ((uint16_t)data[13] << 2) | ((data[14] >> 6) & 0x03);

    Serial.println("\n--- Mesure BME680 ---");

    // Les valeurs ci-dessous sont brutes, à calibrer si besoin
    Serial.print("Température (brute): ");
    Serial.println(adc_temp);

    Serial.print("Pression (brute): ");
    Serial.println(adc_pres);

    Serial.print("Humidité (brute): ");
    Serial.println(adc_hum);

    Serial.print("Résistance gaz (brute): ");
    Serial.println(gas_res);

    Serial.println("Attente 5s...");
    delay(5000);
}
