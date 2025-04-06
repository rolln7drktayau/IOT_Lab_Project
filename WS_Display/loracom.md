# 📡 LoRa_Sender_Receiver_OLED – Communication LoRa + Affichage OLED

Ce montage met en œuvre une communication **LoRa bidirectionnelle** entre deux cartes **Heltec WiFi LoRa 32 V3**. Un module agit comme **émetteur**, en envoyant régulièrement des **données capteurs simulées** (proximité, couleurs RVB), pendant que l’autre les **reçoit**, les **affiche** sur un **écran OLED** et les rend disponibles via une **interface Web**.

Chaque transmission est suivie d’une **attente d’ACK**, permettant de suivre la **fiabilité** du lien LoRa. Le projet utilise la bibliothèque **RadioLib** pour la radio, et **U8g2** pour l’écran OLED (sans dépendance Adafruit).

---

## 🖼️ Schéma du système

```plaintext
[Capteurs]     [LoRa Sender]          [LoRa Receiver]        [WiFi]
  🟢🔴🔵    => TX via LoRa =>   <= RX, envoie ACK <=   => Web Server
               + OLED Stats             + OLED & IP
```

---

## 🛠️ Matériel utilisé

| Élément             | Détails                          |
|---------------------|----------------------------------|
| **Microcontrôleurs**| 2 × Heltec WiFi LoRa 32 V3       |
| **Écrans OLED**     | 2 × OLED 128x64 (SSD1306 I2C)    |
| **Radio LoRa**      | SX1262 intégré                   |
| **Réseau WiFi**     | Requis côté récepteur (Web)      |

---

| Signal OLED       | GPIO utilisé       |
|--------------------|-------------------|
| SDA (écran)        | GPIO17            |
| SCL (écran)        | GPIO18            |
| RST (écran)        | GPIO21            |

---

| Signal LoRa SX1262 | GPIO Heltec       |
|--------------------|-------------------|
| SCK                | GPIO9             |
| MISO               | GPIO11            |
| MOSI               | GPIO10            |
| NSS (CS)           | GPIO8             |
| RST                | GPIO12            |
| DIO1 (IRQ)         | GPIO14            |
| BUSY               | GPIO13            |

---

## 📚 Bibliothèques utilisées

| Bibliothèque       | Rôle                                      |
|--------------------|--------------------------------------------|
| `RadioLib.h`       | Gestion de la radio SX1262 (LoRa)          |
| `SPI.h`            | Bus SPI pour module LoRa                   |
| `U8g2lib.h`        | Affichage graphique OLED (SSD1306)         |
| `Wire.h`           | Communication I2C pour capteurs (si ajout) |
| `WiFi.h`           | Connexion au réseau WiFi côté récepteur    |
| `WebServer.h`      | Création du mini serveur HTTP              |
| `ArduinoJson.h`    | Manipulation des données JSON reçues       |

---

## 🗺️ Carte visuelle des connexions

### 🔌 Connexions physiques – Vue synthétique

```plaintext
📟 Heltec WiFi LoRa 32 V3 (Sender/Receiver)
┌────────────────────────────────────────────┐
│                                            │
│    GPIO41 ────┐                            │
|               |── (I2C - Capteurs - Sender)| 
│    GPIO42 ────┘                            |
│                                            │
│    GPIO17 ─────────────── SDA OLED         │
│    GPIO18 ─────────────── SCL OLED         │
│    GPIO21 ─────────────── RST OLED         │
│                                            │
│    GPIO8  ─────────────── LoRa NSS (CS)    │
│    GPIO9  ─────────────── LoRa SCK         │
│    GPIO10 ────────────── LoRa MOSI         │
│    GPIO11 ────────────── LoRa MISO         │
│    GPIO12 ────────────── LoRa RST          │
│    GPIO13 ────────────── LoRa BUSY         │
│    GPIO14 ────────────── LoRa DIO1 (IRQ)   │
│                                            │
└────────────────────────────────────────────┘
```

🧠 Le module **LoRa est intégré** sur la carte Heltec, mais ces broches sont **exposées** dans le code pour permettre le contrôle via `RadioLib`.

---

### 🎨 Schéma illustré simplifié (ASCII Art)

```plaintext
     ┌────────────────────────────┐
     │     Heltec V3             │
     │                            │
     │  ┌──── OLED 128x64 ─────┐  │
     │  │ SDA  <── GPIO17      │  │
     │  │ SCL  <── GPIO18      │  │
     │  │ RST  <── GPIO21      │  │
     │  └──────────────────────┘  │
     │                            │
     │  ┌── LoRa SX1262 (intégré) │
     │  │ SCK   <── GPIO9         │
     │  │ MOSI  <── GPIO10        │
     │  │ MISO  <── GPIO11        │
     │  │ NSS   <── GPIO8         │
     │  │ RST   <── GPIO12        │
     │  │ BUSY  <── GPIO13        │
     │  │ DIO1  <── GPIO14        │
     │  └────────────────────────┘
     │                            │
     │  [Option Sender uniquement]│
     │  I2C SDA <── GPIO41        │
     │  I2C SCL <── GPIO42        │
     └────────────────────────────┘
```

---

💡 **Conseils montage :**

- Utilise des **résistances de pull-up** (4.7kΩ) si l’OLED ne les intègre pas.
- Vérifie que l’OLED fonctionne en **3.3V** (sinon mettre un convertisseur de niveau).
- Alimente la carte via **USB-C** ou **LiPo**, la consommation reste faible.

---

Souhaites-tu aussi un **fichier image SVG/PNG** de ce schéma ou une **affiche PDF A4** à imprimer ? Je peux te le générer !

---

## ⚙️ Fonctionnement du Sender

1. Simule des mesures capteurs aléatoires (proximité, RVB, clear).
2. Formate les données en **JSON**.
3. Les **envoie en LoRa** via `radio.transmit()`.
4. Passe en mode **réception LoRa** pour attendre un **ACK**.
5. Si ACK reçu dans 1s → incrémente les statistiques.
6. Affiche le résultat et les stats sur **OLED**.

### 📤 Exemple de JSON envoyé

```json
{
  "proximity": 23,
  "clear": 101,
  "red": 58,
  "green": 73,
  "blue": 89
}
```

---

## ⚙️ Fonctionnement du Receiver

1. Attend en **réception LoRa** (`startReceive()`).
2. À la réception :
   - Lit les données JSON.
   - Envoie un **ACK** immédiat avec `radio.transmit("ACK")`.
   - Sauvegarde la chaîne dans `lastRawData`.
   - Met à jour l’écran OLED avec les dernières infos.
3. Sert l’interface Web :
   - `/` = page HTML (peut être enrichie)
   - `/data` = JSON brut actuel

---

## 📶 Web Server intégré

Le **receiver** se connecte à un réseau WiFi existant, puis ouvre un **serveur HTTP** :

| Route         | Contenu                     |
|---------------|-----------------------------|
| `/`           | Page HTML (placeholder)     |
| `/data`       | Dernières données JSON      |

🧠 Utile pour afficher les mesures sur un dashboard externe ou les logger via un script.

---

## 📟 Affichage OLED

Sur **chaque module**, un écran OLED montre l’état actuel :

### Sender

- Statut d’envoi : `Sending...` / `Success!` / `Error`
- Données envoyées (prox, RGB)
- Compteurs de paquets et taux d’ACK

### Receiver

- Adresse IP WiFi
- Dernier JSON reçu
- Attente si rien n’est encore reçu

---

## 📁 Fichiers inclus

- [`sender.ino`](./sender.ino) – Code LoRa + JSON + ACK + OLED côté émetteur
- [`receiver.ino`](./receiver.ino) – Code LoRa + WebServer + OLED côté récepteur

---

## 💬 Extrait console (Sender)

```plaintext
LoRa Sender ready
Data Successfully sent : {"proximity":22,"clear":123,"red":61,"green":71,"blue":91}
ACK received !
```

---

## 💬 Extrait console (Receiver)

```plaintext
Connected to WiFi, IP: 192.168.1.124
Paquet LoRa reçu : {"proximity":22,"clear":123,"red":61,"green":71,"blue":91}
```

---

## 🧠 Remarques techniques

- Le **mode `startReceive()`** de RadioLib permet d’intercepter des messages LoRa sans interruption bloquante.
- L’ACK est une simple string `"ACK"`, transmise dès réception.
- L’I2C logiciel (`SW_I2C`) pour l’OLED permet d’éviter des conflits avec d’autres périphériques I2C.
- Le **taux de réussite** est calculé à partir du nombre de paquets envoyés/acknowledgés.

---

## 🔍 Détails des bibliothèques et broches utilisées

### 📦 `RadioLib.h`

📡 Bibliothèque de haut niveau pour la gestion des modules radio LoRa (ici, **SX1262** intégré à la Heltec V3).

- Permet les fonctions :
  - `begin()`, `transmit()`, `startReceive()`
  - Gestion des ACK, RSSI, erreurs, etc.
- Abstraction complète du protocole LoRa (et autres : FSK, RTTY, etc.)
- Utilise le **bus SPI**, défini manuellement :

```cpp
#define LORA_SCK     9
#define LORA_MISO   11
#define LORA_MOSI   10
#define LORA_SS      8
#define LORA_RST    12
#define LORA_DIO0   14
#define LORA_BUSY   13

SX1262 radio = new Module(LORA_SS, LORA_DIO0, LORA_RST, LORA_BUSY);
```

🧠 La **broche BUSY** est obligatoire pour SX1262, contrairement à SX127x.

---

### 📦 `SPI.h`

🧪 Permet la communication avec le module **LoRa** via le bus **SPI** :

- Utilisé implicitement par `RadioLib`
- La configuration SPI est faite en assignant les broches manuellement (voir ci-dessus).

---

### 📦 `U8g2lib.h`

🎨 Pour l’affichage OLED en I2C logiciel (voir section dédiée plus haut).

- Initialisation typique (valable pour les deux cartes) :

```cpp
U8G2_SSD1306_128X64_NONAME_F_SW_I2C u8g2(
  U8G2_R0,
  /* clock=*/ 18,
  /* data=*/ 17,
  /* reset=*/ 21
);
```

🧠 Ce choix évite les conflits avec **Wire/I2C matériel** (GPIO41/42).

---

### 📦 `Wire.h` *(utilisé uniquement dans le Sender)*

🔌 Gestion du bus I2C matériel, utilisé ici **si des capteurs réels sont ajoutés**.

```cpp
#define SDA_PIN 41
#define SCL_PIN 42
```

🧠 Pas encore exploité dans le code, mais réservé pour l’ajout futur de capteurs physiques (ex. TMG39931, TCS34725...).

---

### 📦 `WiFi.h` *(Receiver uniquement)*

🌐 Connecte le module Heltec au réseau WiFi local :

```cpp
const char *ssid = "RCT GS22 U";
const char *password = "xymp4417";
```

🧠 Nécessaire uniquement pour le **receiver**, qui héberge un **serveur HTTP**.

---

### 📦 `WebServer.h` *(Receiver uniquement)*

🖥️ Crée un serveur web minimal embarqué :

```cpp
WebServer server(80);
```

- Sert des routes `/` et `/data` (cf. section "Web Server intégré").
- Utilisé de façon non bloquante dans `loop()` pour afficher ou consulter les dernières données.

---

### 📦 `ArduinoJson.h`

🧬 Gère la sérialisation et le parsing des données JSON échangées par LoRa :

- **Sender** : utilise `serializeJson()` pour formater les mesures
- **Receiver** : utilise `deserializeJson()` pour extraire les valeurs reçues

🧠 Très léger et rapide, parfait pour microcontrôleurs.

---

### 📊 Récapitulatif des broches utilisées

#### 🔹 Communes aux deux cartes

| Fonction         | Broche      |
|------------------|-------------|
| OLED SDA         | GPIO17      |
| OLED SCL         | GPIO18      |
| OLED RST         | GPIO21      |
| LoRa SCK         | GPIO9       |
| LoRa MISO        | GPIO11      |
| LoRa MOSI        | GPIO10      |
| LoRa SS (CS)     | GPIO8       |
| LoRa RST         | GPIO12      |
| LoRa DIO1 (IRQ)  | GPIO14      |
| LoRa BUSY        | GPIO13      |

#### 🔹 Spécifiques au Sender

| Fonction              | Broche       |
|-----------------------|--------------|
| I2C matériel SDA      | GPIO41       |
| I2C matériel SCL      | GPIO42       |

🧠 L’I2C matériel est uniquement configuré pour **futurs capteurs**.

---

## 🔗 Ressources utiles

- [RadioLib Documentation](https://github.com/jgromes/RadioLib)
- [U8g2 Wiki](https://github.com/olikraus/u8g2/wiki)
- [ArduinoJson](https://arduinojson.org/)
- [Heltec LoRa V3 Docs](https://docs.heltec.org/)

---
