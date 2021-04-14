#ifndef __BME280_H
#define __BME280_H

#include "Wire.h"
#include "DFRobot_BME280.h"

class BME280 : public DFRobot_BME280_IIC {

public:
  BME280() : DFRobot_BME280_IIC(nullptr, 0) {};
  BME280(TwoWire *pWire, uint8_t addr);

  void setI2CInterface(TwoWire *pWire);
  void setAddr(uint8_t addr);
};


#endif /* end of include guard: __BME280_H */
