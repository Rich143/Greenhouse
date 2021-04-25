#include "WaterPump.h"
#include "Power_Controller.h"

status_t WaterPump::init()
{
    pinMode(_pump_enable_pin, OUTPUT);

    return gPowerController.init();
}


status_t WaterPump::turnOn()
{
    status_t rc =
        gPowerController.setPowerChannel(PowerController::POWER_CHANNEL_9V_12V,
                                         true);
    if (rc != STATUS_OK) {
      return rc;
    }

    digitalWrite(_pump_enable_pin, HIGH);

    return STATUS_OK;
}

status_t WaterPump::turnOff()
{
    digitalWrite(_pump_enable_pin, LOW);

    status_t rc =
        gPowerController.setPowerChannel(PowerController::POWER_CHANNEL_9V_12V,
                                         false);
    if (rc != STATUS_OK) {
      return rc;
    }

    return STATUS_OK;
}
