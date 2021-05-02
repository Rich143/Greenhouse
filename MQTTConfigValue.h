#ifndef MQTTCONFIGVALUE_H_3AHQU2BD
#define MQTTCONFIGVALUE_H_3AHQU2BD

#include "Adafruit_MQTT.h"
#include "Status.h"

//! \class MQTTConfigValue
//\brief Class to manage updating a config value over MQTT
class MQTTConfigValue
{
    public:
        /**
         * @brief Constructor
         *
         * @param mqtt Pointer to the MQTT instance
         * @param configMQTTFeedName The feed name containing the config values
         * (need to prepend username)
         * @param configMQTTFeedNameGet The feed name with /get appended
         * (need to prepend username)
         */
        MQTTConfigValue(Adafruit_MQTT *mqtt, const char *configMQTTFeedName, const char *configMQTTFeedNameGet);

        status_t init();

        status_t updateValue();

        double getValueDouble();

        std::pair<uint8_t *, uint8_t> getValueBuffer();

        bool getValueOnOff();

    protected:
        enum {
            SUBSCRIPTION_READ_TIMEOUT_MS = 50,
            SUBSCRIPTION_READ_NUM_TRIES = 10,
        };

        Adafruit_MQTT *_mqtt;
        Adafruit_MQTT_Subscribe _value_feed;
        Adafruit_MQTT_Publish _get_feed;
};

#endif /* end of include guard: MQTTCONFIGVALUE_H_3AHQU2BD */
