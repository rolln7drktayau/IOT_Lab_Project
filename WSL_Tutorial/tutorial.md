# 🛠️ Installation et configuration de l’IDE Arduino dans WSL

Ce guide détaille comment installer et utiliser **l’IDE Arduino sous WSL (Windows Subsystem for Linux)**, y compris l’accès aux **ports USB pour la programmation directe** depuis l’environnement Linux.

---

## ✅ Prérequis

- Avoir **WSL2** installé sur Windows 10/11
- Avoir une **distribution Ubuntu** fonctionnelle
- Disposer de l’outil `usbipd` installé côté Windows (disponible nativement via Windows Features)

---

## 📦 Installation de l’IDE Arduino (version Linux)

### 1. Télécharger l’IDE Arduino

Rends-toi sur le site officiel :

🔗 <https://www.arduino.cc/en/software>

Télécharge la version **Linux 64 bits** (au format `.tar.xz`)
ou utiliser la commande :

```bash
wget https://downloads.arduino.cc/arduino-ide/arduino-ide_2.2.1_Linux_64bit.tar.xz
```

### 2. Extraire et lancer

Dans WSL :

```bash
cd ~/Downloads
tar -xf arduino-ide_*.tar.xz
cd arduino-ide_*
./arduino-ide
```

🧠 L’IDE graphique s’ouvrira via X11. Assure-toi d’avoir un **serveur graphique** fonctionnel sur Windows (comme [VcXsrv](https://sourceforge.net/projects/vcxsrv/) ou [WSLg](https://learn.microsoft.com/en-us/windows/wsl/tutorials/gui-apps)).

---

## ⚙️ Accès aux ports USB sous WSL

Par défaut, WSL2 ne voit pas les périphériques USB série. Voici comment les activer via `usbip`.

---

### 🔧 Étapes côté WSL (Bash)

```bash
sudo apt update
sudo apt install linux-tools-generic hwdata
sudo update-alternatives --install /usr/local/bin/usbip usbip /usr/lib/linux-tools/*/usbip 20
```

---

### ⚡ Étapes côté Windows (PowerShell Admin)

#### 1. Liste les périphériques USB connectés

```powershell
usbipd list
```

#### 2. Repère le **bus ID** de ton Arduino (ex. `4-1`), puis attache-le à WSL

```powershell
usbipd bind --busid=4-1
usbipd attach --wsl --busid=4-1
```

---

### 📂 Vérifier dans WSL (Bash)

```bash
ls /dev/ttyUSB* /dev/ttyACM*
```

Tu devrais voir ton port série (ex. `/dev/ttyACM0`).

Ajoute ton utilisateur au groupe `dialout` pour accéder au port sans sudo :

```bash
sudo usermod -a -G dialout $USER
```

Puis redémarre la session WSL (`exit` puis relance `wsl`).

---

## 🚀 Lancer Arduino IDE

Reviens dans le dossier extrait :

```bash
cd ~/Downloads/arduino-ide_*/
./arduino-ide
```

Dans l’IDE, va dans **Tools > Port** et sélectionne `/dev/ttyACM0` ou équivalent.

---

## 🧠 Astuce

Ajoute un alias dans `.bashrc` pour le lancer facilement :

```bash
echo "alias arduino='~/Downloads/arduino-ide_*/arduino-ide'" >> ~/.bashrc
source ~/.bashrc
```

Tu peux alors le lancer avec :

```bash
arduino
```

---

## 🧪 Test rapide

Branche une carte Arduino ou Heltec, ouvre un exemple (`File > Examples > Blink`), sélectionne le port, et clique sur **Upload**.

---

## 🖇️ Ressources utiles

- [Arduino Linux Install Guide](https://support.arduino.cc/hc/en-us/articles/360019833020-How-to-install-the-Arduino-IDE-on-Linux)
- [usbipd-win](https://github.com/dorssel/usbipd-win)
- [WSL2 + USB Guide](https://learn.microsoft.com/en-us/windows/wsl/connect-usb)

---
