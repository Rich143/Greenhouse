# My makefile
SKETCH = ./Greenhouse.ino

CHIP=esp32
UPLOAD_PORT=/dev/cu.usbserial-015E0000

include $(HOME)/makeEspArduino/makeEspArduino.mk
