# My makefile
SKETCH = ./ESP32_Bat_LC709203F_Web_server.ino

CHIP=esp32
UPLOAD_PORT=/dev/cu.usbserial-015E0000

include $(HOME)/makeEspArduino/makeEspArduino.mk
