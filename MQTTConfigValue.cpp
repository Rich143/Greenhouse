#include "MQTTConfigValue.h"
#include "Logger.h"

MQTTConfigValue::MQTTConfigValue(Adafruit_MQTT *mqtt,
                                 const char *configMQTTFeedName,
                                 const char *configMQTTFeedNameGet) :
    _value_feed(mqtt, configMQTTFeedName),
    _get_feed(mqtt, configMQTTFeedNameGet),
    _mqtt(mqtt)
{}

status_t MQTTConfigValue::init()
{
    if (!_mqtt->subscribe(&_value_feed)) {
        LOG_ERROR("Failed to subscribe to mqtt feed");
        return STATUS_FAIL;
    }

    return STATUS_OK;
}

status_t MQTTConfigValue::updateValue()
{
    Adafruit_MQTT_Subscribe *subscription;

    if (!_get_feed.publish('\0')) {
        LOG_ERROR("Failed to publish to get feed");
        return STATUS_FAIL;
    }

    for (int i = 0; i < SUBSCRIPTION_READ_NUM_TRIES; ++i) {
        if ((subscription = _mqtt->readSubscription(SUBSCRIPTION_READ_TIMEOUT_MS))) {
            if (subscription == &_value_feed) {
                return STATUS_OK;
            }
        }
    }

    LOG_ERROR("Didn't get new value from value feed for subscription");

    return STATUS_FAIL;
}

double MQTTConfigValue::getValueDouble()
{
    double data = atof((char *)_value_feed.lastread);

    return data;
}

std::pair<uint8_t *, uint8_t> MQTTConfigValue::getValueBuffer()
{
    std::pair<uint8_t *, uint8_t> bufferPair = std::make_pair(_value_feed.lastread,
                                                              _value_feed.datalen);

    return bufferPair;
}

bool MQTTConfigValue::getValueOnOff()
{
      if (strcmp((char *)_value_feed.lastread, "ON") == 0) {
          return true;
      } else if (strcmp((char *)_value_feed.lastread, "OFF") == 0) {
          return false;
      } else {
          LOG_ERROR("Got unknown value "
                    + String((char *)_value_feed.lastread)
                    + " from feed " + String(_value_feed.topic));
          return false;
      }
} 
