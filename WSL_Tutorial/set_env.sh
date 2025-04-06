# Bash 
sudo apt update
sudo apt install linux-tools-generic hwdata
sudo update-alternatives --install /usr/local/bin/usbip usbip /usr/lib/linux-tools/*/usbip 20

# PS
usbipd list
usbipd bind --busid=4-1
usbipd attach --wsl --busid=4-1

# Bash
ls /dev/ttyUSB* /dev/ttyACM*
sudo usermod -a -G dialout $USER