#include <Wire.h>
#include "HT_SSD1306Wire.h"

// Configuration de l'écran OLED
static SSD1306Wire display(0x3c, 500000, SDA_OLED, SCL_OLED, GEOMETRY_128_64, RST_OLED);

void setup()
{
  // Initialisation de la communication série (optionnel, mais utile pour le débogage)
  Serial.begin(115200);

  // Active l'alimentation du module Vext (spécifique à certaines cartes)
  pinMode(Vext, OUTPUT);
  digitalWrite(Vext, LOW);

  // Délai court pour stabilisation
  delay(100);

  // Initialise l'écran OLED
  display.init();

  // Prépare l'écran pour l'affichage
  display.clear();
}

void loop()
{
  // Efface l'écran avant de dessiner
  display.clear();

  // Définit l'alignement du texte à gauche
  display.setTextAlignment(TEXT_ALIGN_LEFT);

  // Premier Hello World en petite taille (10)
  display.setFont(ArialMT_Plain_10);
  display.drawString(0, 0, "Hello world !");

  // Deuxième Hello World en taille moyenne (16)
  display.setFont(ArialMT_Plain_16);
  display.drawString(0, 20, "Hello world !");

  // Troisième Hello World en grande taille (24)
  display.setFont(ArialMT_Plain_24);
  display.drawString(0, 40, "Hello world !");

  // Affiche le contenu sur l'écran
  display.display();

  // Attend 5 secondes avant de recommencer
  delay(5000);
}