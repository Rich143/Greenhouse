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

    // Wait for the output capacitor on the 9/12V rail to charge up
    // There was issues turning on the pump without the capacitor, so the
    // capacitor should be charged up to handle spikes in current draw on
    // startup
    delay(100);

    pinMode(_pump_enable_pin, OUTPUT);
    digitalWrite(_pump_enable_pin, HIGH);

    return STATUS_OK;
}

status_t WaterPump::turnOff()
{
    // Bit of a hack, there seems to be some weird current path created when we
    // pull this pin low that causes the motor controller LED to stay on. My
    // best guess is since the 12V output is turned off with a low side switch,
    // somehow pulling this pin low acts as the ground path for the 12V output.
    // By setting the pin as input, we put it in high impedance mode which
    // seems to mostly (but not fully) fix this issue
    pinMode(_pump_enable_pin, INPUT);

    status_t rc =
        gPowerController.setPowerChannel(PowerController::POWER_CHANNEL_9V_12V,
                                         false);
    if (rc != STATUS_OK) {
      return rc;
    }

    return STATUS_OK;
}
