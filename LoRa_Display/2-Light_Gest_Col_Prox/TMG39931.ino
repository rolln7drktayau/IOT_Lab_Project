#include <Wire.h>
#include <U8g2lib.h>

#define SDA_PIN 41  // J3 Pin 8 (GPIO41)
#define SCL_PIN 42  // J3 Pin 7 (GPIO42)
#define OLED_SDA 17 // Default OLED SDA pin for Heltec WiFi LoRa 32 V3
#define OLED_SCL 18 // Default OLED SCL pin for Heltec WiFi LoRa 32 V3
#define OLED_RST 21 // Default OLED RST pin for Heltec WiFi LoRa 32 V3

// TMG39931 I2C address
#define TMG39931_I2C_ADDR 0x39

// TMG39931 registers
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

// Create U8g2 display object - check the constructor for your specific display
// This is for SSD1306 128x64 display with hardware I2C
U8G2_SSD1306_128X64_NONAME_F_SW_I2C u8g2(U8G2_R0, OLED_SCL, OLED_SDA, OLED_RST);

bool pauseReading = false;
unsigned long lastUpdateTime = 0;
const int updateInterval = 2000; // Update every 2 seconds

// Variables to store sensor data
uint8_t proximity = 0;
uint16_t clear = 0, red = 0, green = 0, blue = 0;
float rNorm = 0, gNorm = 0, bNorm = 0;
float luminousFlux = 0;

// Function to write to a register
void writeRegister(byte reg, byte value)
{
  Wire.beginTransmission(TMG39931_I2C_ADDR);
  Wire.write(reg);
  Wire.write(value);
  Wire.endTransmission();
}

// Function to read from a register
byte readRegister(byte reg)
{
  Wire.beginTransmission(TMG39931_I2C_ADDR);
  Wire.write(reg);
  Wire.endTransmission();

  Wire.requestFrom(TMG39931_I2C_ADDR, 1);
  return Wire.read();
}

// Function to initialize the sensor
bool initializeTMG39931()
{
  // Check sensor ID
  byte id = readRegister(TMG39931_ID);
  Serial.print("Sensor ID: 0x");
  Serial.println(id, HEX);

  // According to documentation, ID should be 0xA8
  if (id != 0xA8)
  {
    Serial.println("WARNING: Incorrect sensor ID, but continuing...");
  }

  // Disable sensor during configuration
  writeRegister(TMG39931_ENABLE, 0x00);
  delay(10);

  // Configure ADC integration time
  writeRegister(TMG39931_ATIME, 0xFF); // 2.7ms
  delay(10);

  // Configure persistence
  writeRegister(TMG39931_PERS, 0x00); // Each cycle generates an interrupt
  delay(10);

  // Configure gain
  writeRegister(TMG39931_CONTROL, 0x00); // 1x gain
  delay(10);

  // Enable sensor (Power ON + color sensor + proximity sensor)
  writeRegister(TMG39931_ENABLE, 0x0F); // 0x0F = PON + AEN + PEN
  delay(10);

  return true;
}

// Function to read sensor data
void readSensorData()
{
  // Read proximity value
  proximity = readRegister(TMG39931_PDATA);

  // Read color values
  clear = readRegister(TMG39931_CDATAL) | (readRegister(TMG39931_CDATAH) << 8);
  red = readRegister(TMG39931_RDATAL) | (readRegister(TMG39931_RDATAH) << 8);
  green = readRegister(TMG39931_GDATAL) | (readRegister(TMG39931_GDATAH) << 8);
  blue = readRegister(TMG39931_BDATAL) | (readRegister(TMG39931_BDATAH) << 8);

  // Calculate luminous flux (simple approximation)
  luminousFlux = 0.299 * red + 0.587 * green + 0.114 * blue;

  // Calculate normalized RGB values
  float total = red + green + blue;
  if (total > 0)
  {
    rNorm = red / total;
    gNorm = green / total;
    bNorm = blue / total;
  }
  else
  {
    rNorm = gNorm = bNorm = 0;
  }
}

// Function to update the display
void updateDisplay()
{
  char buffer[32];

  u8g2.clearBuffer();

  // Display title
  u8g2.setFont(u8g2_font_6x10_tf);
  u8g2.drawStr(22, 10, "TMG39931 Sensor");
  u8g2.drawHLine(0, 12, 128);

  // Display RGB values
  sprintf(buffer, "R:%d G:%d B:%d", red, green, blue);
  u8g2.drawStr(0, 24, buffer);

  // Display Clear and Proximity values
  sprintf(buffer, "C:%d Prox:%d", clear, proximity);
  u8g2.drawStr(0, 36, buffer);

  // Display normalized RGB values
  sprintf(buffer, "rN:%.2f gN:%.2f bN:%.2f", rNorm, gNorm, bNorm);
  u8g2.drawStr(0, 48, buffer);

  // Display luminous flux
  sprintf(buffer, "Flux: %.1f", luminousFlux);
  u8g2.drawStr(0, 60, buffer);

  // Show paused status if applicable
  if (pauseReading)
  {
    u8g2.drawStr(90, 60, "PAUSED");
  }

  u8g2.sendBuffer();
}

void displayMessage(const char *line1, const char *line2 = NULL, const char *line3 = NULL)
{
  u8g2.clearBuffer();
  u8g2.setFont(u8g2_font_6x10_tf);

  u8g2.drawStr(0, 10, line1);

  if (line2)
  {
    u8g2.drawStr(0, 25, line2);
  }

  if (line3)
  {
    u8g2.drawStr(0, 40, line3);
  }

  u8g2.sendBuffer();
}

void setup()
{
  // Initialize Serial
  Serial.begin(115200);
  while (!Serial)
    delay(10);

  Serial.println("\nStarting TMG39931 program...");

  // Reset OLED
  pinMode(OLED_RST, OUTPUT);
  digitalWrite(OLED_RST, LOW);
  delay(20);
  digitalWrite(OLED_RST, HIGH);

  // Initialize display
  u8g2.begin();
  u8g2.setFont(u8g2_font_6x10_tf);

  displayMessage("Starting TMG39931...", "Initializing I2C...");

  // Initialize I2C for sensor
  Wire.begin(SDA_PIN, SCL_PIN);
  Wire.setClock(100000); // Reduced frequency for more stability

  // Check for sensor on I2C bus
  Wire.beginTransmission(TMG39931_I2C_ADDR);
  byte error = Wire.endTransmission();

  if (error == 0)
  {
    displayMessage("TMG39931 detected!", "Initializing...");
    Serial.println("TMG39931 sensor detected at address 0x39!");

    if (initializeTMG39931())
    {
      displayMessage("TMG39931 detected!", "Sensor initialized!", "First reading in 3s...");
      Serial.println("TMG39931 sensor initialized successfully!");
    }
    else
    {
      displayMessage("TMG39931 detected!", "Initialization failed!");
      Serial.println("Failed to initialize sensor!");
    }
  }
  else
  {
    displayMessage("Sensor not found!", "Check wiring.");
    Serial.println("TMG39931 sensor not detected. Check wiring.");
    while (1)
    {
      delay(1000);
      if (Serial.available() && Serial.read() == 'r')
      {
        ESP.restart();
      }
    }
  }

  Serial.println("\nSystem ready. Available commands:");
  Serial.println("p - Pause/Resume measurements");
  Serial.println("r - Restart ESP");

  delay(3000);
}

void loop()
{
  // Check if a command has been received
  if (Serial.available())
  {
    char cmd = Serial.read();
    if (cmd == 'r')
    {
      Serial.println("Restarting system...");
      displayMessage("Restarting...");
      delay(1000);
      ESP.restart();
    }
    else if (cmd == 'p')
    {
      pauseReading = !pauseReading;
      if (pauseReading)
      {
        Serial.println("\n*** MEASUREMENTS PAUSED ***");
        Serial.println("Press 'p' to resume");
      }
      else
      {
        Serial.println("\n*** MEASUREMENTS RESUMED ***");
      }
      updateDisplay(); // Update display to show pause status
    }
  }

  // If not paused, take measurements at regular intervals
  if (!pauseReading && (millis() - lastUpdateTime >= updateInterval))
  {
    readSensorData();
    updateDisplay();

    // Print detailed data to Serial
    Serial.println("\n------ New Measurements ------");
    Serial.print("Proximity: ");
    Serial.println(proximity);
    Serial.println("Color values:");
    Serial.print("Red: ");
    Serial.println(red);
    Serial.print("Green: ");
    Serial.println(green);
    Serial.print("Blue: ");
    Serial.println(blue);
    Serial.print("Clear: ");
    Serial.println(clear);
    Serial.print("Luminous Flux: ");
    Serial.println(luminousFlux);

    if (red + green + blue > 0)
    {
      Serial.print("Normalized RGB: (");
      Serial.print(rNorm, 3);
      Serial.print(", ");
      Serial.print(gNorm, 3);
      Serial.print(", ");
      Serial.print(bNorm, 3);
      Serial.println(")");
    }

    lastUpdateTime = millis();
  }

  // Small delay to prevent excessive CPU usage
  delay(50);
}