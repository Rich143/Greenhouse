#include "Arduino.h"
#include "Power_Controller.h"

#define PIN_9V_12V_ENABLE 26
#define PIN_3V3_ENABLE 23

PowerController gPowerController;

PowerController::PowerController()
{
    initialized = false;
}

power_controller_status_t PowerController::setPowerChannel(PowerChannel channel,
                                                           bool enable)
{
    switch (channel) {
        case POWER_CHANNEL_3V3:
            digitalWrite(PIN_3V3_ENABLE, enable);
            break;
        case POWER_CHANNEL_9V_12V:
            digitalWrite(PIN_9V_12V_ENABLE, enable);
            break;
        default:
            Serial.print("Set Power Channel: invalid channel ");
            Serial.println(channel);
            return POWER_CONTROLLER_INVALID_PARAMS;
            break;
    }

    return POWER_CONTROLLER_OK;
}

power_controller_status_t PowerController::init()
{
    if (!initialized) {
        pinMode(PIN_9V_12V_ENABLE, OUTPUT);
        pinMode(PIN_3V3_ENABLE, OUTPUT);
    }

    return POWER_CONTROLLER_OK;
}
