# 🦩 IOT_Lab_Project – HELTEC WIFI LoRa 32V3

## 📌 Auteurs : Les Flamants Roses🦩

### Roland Cédric TAYO  

[![LinkedIn](https://img.shields.io/badge/LinkedIn-0077B5?style=for-the-badge&logo=linkedin&logoColor=white)](https://www.linkedin.com/in/rct/)
[![GitHub](https://img.shields.io/badge/GitHub-100000?style=for-the-badge&logo=github&logoColor=white)](https://github.com/rolln7drktayau/)

## 📝 Description

Ce projet présente l'exploitation de différents capteurs à l'aide d'un module de type HELTEC WIFI LORA 32V3. Nous avons proposé 3 méthodes d'affichage des résultats, à savoir :

- **Ide_Display** : Affichage via une interface de développement classique de type ARDUINO.
- **LoRa_Display** : Transmission et affichage des données via la technologie **LoRa**, sur l'écran OLED du module.
- **WS_Display** : Utilisation d'un **WebServer** alimenté via la communication radio.

Chaque méthode contient trois modules de capteurs différents ainsi qu'un fichier `Hello_World` pour tester la connectivité ou le bon fonctionnement de la méthode d'affichage.

## 📁 Structure du projet

```markdown
IOT_Lab_Project
├── assets
├── Ide_Display
│   ├── 1-Temp_Air-Press_Hum_Air-Qual
│   ├── 2-Light_Gest_Col_Prox
│   ├── 3-HeartBeat_Sensor
│   └── Hello_World
├── LoRa_Display
│   ├── 1-Temp_Air-Press_Hum_Air-Qual
│   ├── 2-Light_Gest_Col_Prox
│   ├── 3-HeartBeat_Sensor
│   └── Hello_World
├── WS_Display
├── WSL_Tutorial
└── README.md
```

## 🔍 Détail des affichages

- [Ide_Display](./Ide_Display) : Utilisation classique de l'IDE pour lire les valeurs des capteurs.
- [LoRa_Display](./LoRa_Display) : Affichage des capteurs utilisant le protocole LoRa pour la transmission longue portée.
- [WS_Display](./WS_Display) : Utilisation d'un WebServer pour l'envoi en temps réel des mesures des capteurs. **Seul le capteur TMG39931 sera utilisé pour cette méthode d'affichage**, qui mesure la lumière ambiante, la couleur et les gestes, soit le cas 2.

Chaque répertoire contient les modules suivants :

- `1-Temp_Air-Press_Hum_Air-Qual` : Température, pression, humidité et qualité de l'air.
- `2-Light_Gest_Col_Prox` : Lumière, gestes, couleur et proximité.
- `3-HeartBeat_Sensor` : Capteur de fréquence cardiaque.
- `Hello_World` : Test basique de fonctionnement du canal de communication.

---

## 💻 Installation WSL (Windows Subsystem for Linux)

Si vous rencontrez des problèmes liés aux pilotes sur Windows, vous pouvez consulter notre tutoriel détaillé pour installer et configurer correctement votre environnement de développement avec **WSL (Windows Subsystem for Linux)**. Le tutoriel couvre l'installation des outils nécessaires, la configuration des périphériques USB et la gestion des connexions série avec l'IDE Arduino sous WSL.

Vous pouvez accéder au tutoriel complet [ici](./WSL_Tutorial).

---

## 📡 WS_Display - Serveur de monitoring via communication radio LoRa

Le dossier WS_Display contient l'implémentation d'un serveur de monitoring qui exploite la communication radio entre deux modules LoRa. Le système se compose de :

- Un fichier `sender.ino` qui collecte les données du capteur TMG39931 et les transmet via LoRa
- Un fichier `receiver.ino` qui reçoit les données et les affiche sur un serveur web
- Un fichier `index.html` pour l'interface web de visualisation des données
- Des captures d'écrans dans le dossier `assets` montrant le résultat du monitoring

Cette implémentation permet de visualiser en temps réel les données du capteur transmises via la communication radio LoRa sur une interface web accessible.

## 📊 Captures d'écran du WS_Display

Voici les captures d'écran montrant l'interface de monitoring via la communication radio LoRa :

### TMG39931 - Monitoring LoRa - Vue 1

![Monitoring LoRa Vue 1](./WS_Display/assets/TMG39931%20-%20Monitoring%20LoRa%20-%20IMG-1.png)

### TMG39931 - Monitoring LoRa - Vue 2

![Monitoring LoRa Vue 2](./WS_Display/assets/TMG39931%20-%20Monitoring%20LoRa%20-%20IMG-2.png)

Ces captures montrent l'interface web qui affiche les données du capteur TMG39931 (lumière ambiante, couleur et proximité) transmises via la communication radio LoRa entre les deux modules HELTEC WIFI LoRa 32V3.

## 👥 Contributeurs

### Sorore BENABIB

---

[![LinkedIn](https://img.shields.io/badge/LinkedIn-0077B5?style=for-the-badge&logo=linkedin&logoColor=white)](https://www.linkedin.com/in/benabid-sorore-0b0929a2/)

---
