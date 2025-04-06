#include <SPI.h>
#include <LoRa.h>

void setup() {
  Serial.begin(115200);
}

void loop() {
  // Envoi d'un message
  Serial.println("Envoi d'un message : Hello World");
  delay(3000); // Envoi toutes les 5 secondes
}
