#include "BME280.h"

BME280::BME280(TwoWire *pWire, uint8_t addr) : DFRobot_BME280_IIC(pWire, addr) {}

void BME280::setAddr(uint8_t addr) {
    _addr = addr;
}

void BME280::setI2CInterface(TwoWire *pWire) {
    _pWire = pWire;
}
