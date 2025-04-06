# 🌐 Hello World LoRa – Envoi de message sur le Serial Monitor avec LoRa

Ce montage utilise le module **LoRa** pour envoyer un message via la communication sans fil. Le message est envoyé toutes les **3 secondes** à l'aide d'une carte compatible avec la bibliothèque **LoRa** via l'interface **SPI**.

---

## 📷 Image du module

![Image du module](./hw.png)

---

## 🛠️ Matériel utilisé

| Élément             | Détails                           |
|---------------------|-----------------------------------|
| **Microcontrôleur** | Compatible avec SPI (par exemple ESP32, Arduino) |
| **Module LoRa**     | LoRa Module (par exemple RFM95, SX1278) |
| **Interface**       | SPI                               |

| Signal LoRa  | Broche Microcontrôleur (Exemple ESP32) |
|--------------|--------------------------------------|
| MISO         | GPIO19 (ESP32)                      |
| MOSI         | GPIO23 (ESP32)                      |
| SCK          | GPIO18 (ESP32)                      |
| SS           | GPIO5  (ESP32)                      |

---

## 📚 Bibliothèques utilisées

| Bibliothèque    | Utilisation             |
|------------------|-------------------------|
| `SPI.h`          | Communication SPI       |
| `LoRa.h`         | Communication LoRa      |

---

## ⚙️ Fonctionnement

Le programme :

1. Initialise la communication **SPI** pour utiliser le module **LoRa**.
2. Configure le module LoRa pour envoyer des messages à une fréquence de 3 secondes.
3. Envoie un message "Hello World" via LoRa.
4. Affiche le message envoyé dans le moniteur série.

Le code ne reçoit pas de message, il se concentre uniquement sur l'envoi.

---

## 📁 Fichiers inclus

- [`hw.ino`](./hw.ino) – Code complet en C++ (Arduino)
- [`hw.png`](./hw.png) – Image du module LoRa utilisé dans le projet

---

## 💬 Exemple de sortie console

```plaintext
Envoi d'un message : Hello World
Envoi d'un message : Hello World
Envoi d'un message : Hello World
```

---

## 🧠 Remarques

- Ce projet est un **exemple de base** pour envoyer des messages via LoRa. Il est possible d'étendre ce projet pour recevoir des messages, établir une communication bidirectionnelle, ou ajouter des fonctionnalités comme le cryptage ou la gestion des erreurs.
- L'intervalle entre les envois est actuellement de **3 secondes**, mais il peut être ajusté en modifiant la valeur dans le `delay(3000);`.

---

## 🧩 Initialisation dans ce projet

```cpp
#include <SPI.h>
#include <LoRa.h>

void setup() {
  Serial.begin(115200); // Initialisation du moniteur série
}

void loop() {
  // Envoi d'un message
  Serial.println("Envoi d'un message : Hello World");
  delay(3000); // Envoi toutes les 3 secondes
}
```

Ce projet montre la simplicité d'envoi de messages via LoRa. L'initialisation du module et l'envoi d'un message peuvent être facilement intégrés dans des projets plus complexes nécessitant une communication sans fil longue portée.

---

### 🔗 Ressources utiles

- [Documentation officielle LoRa](https://github.com/sandeepmistry/arduino-LoRa)
- [Exemples d'utilisation de LoRa](https://www.arduino.cc/en/Reference/LoRa)

---
