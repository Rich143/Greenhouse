#ifndef POWER_CONTROLLER_H_HV0ZKTDT
#define POWER_CONTROLLER_H_HV0ZKTDT

enum power_controller_status_t {
    POWER_CONTROLLER_OK,
    POWER_CONTROLLER_INVALID_PARAMS,
    POWER_CONTROLLER_FAIL,
};

class PowerController {
    public:

    enum PowerChannel {
        POWER_CHANNEL_3V3,
        POWER_CHANNEL_9V_12V,
        POWER_CHANNEL_NUM_CHANNELS
    };

    PowerController();

    power_controller_status_t init();

    power_controller_status_t setPowerChannel(PowerChannel channel, bool enable);

    private:

    bool initialized;
};

// Global power controller instance
extern PowerController gPowerController;

#endif /* end of include guard: POWER_CONTROLLER_H_HV0ZKTDT */
