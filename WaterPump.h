#ifndef WATERPUMP_H_IBMFXATB
#define WATERPUMP_H_IBMFXATB

#include "Arduino.h"
#include "Status.h"

#define WATERPUMP_DEFAULT_PUMP_ENABLE_PIN 35

/*!
 * \class WaterPump
 * \brief Water Pump Control
 */
class WaterPump
{
public:
    WaterPump():
        _pump_enable_pin(WATERPUMP_DEFAULT_PUMP_ENABLE_PIN) {};

    status_t init();

    status_t turnOn();

    status_t turnOff();

protected:
    uint32_t _pump_enable_pin;
};

#endif /* end of include guard: WATERPUMP_H_IBMFXATB */
