#include "Arduino.h"
#include "Power_Controller.h"
#include "Logger.h"

#define PIN_9V_12V_ENABLE 26
#define PIN_3V3_ENABLE 23

PowerController gPowerController;

PowerController::PowerController()
{
    initialized = false;
}

status_t PowerController::setPowerChannel(PowerChannel channel,
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
            LOG_WARN("Set Power Channel: invalid channel %d", channel);
            return STATUS_INVALID_PARAMS;
            break;
    }

    return STATUS_OK;
}

status_t PowerController::init()
{
    if (!initialized) {
        pinMode(PIN_9V_12V_ENABLE, OUTPUT);
        pinMode(PIN_3V3_ENABLE, OUTPUT);
    }

    return STATUS_OK;
}
