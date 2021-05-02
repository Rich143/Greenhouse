#ifndef SYSTEMMANAGER_H_HO5YVCME
#define SYSTEMMANAGER_H_HO5YVCME

#include <WiFiClientSecure.h>
#include "Adafruit_MQTT.h"
#include "Adafruit_MQTT_Client.h"
#include "Status.h"
#include "Sensors.h"
#include "ConfigValue.h"
#include "MQTTConfigValue.h"

/*! \class SystemManager
 *  \brief System Manager manages the state of the greenhouse
 *
 */
class SystemManager
{
public:
    SystemManager();

    void run();

    void init();

protected:
    bool shouldWater();

    status_t updateAndPublishSensors();

    status_t initMQTTConfigValues();

    status_t initWifiMQTT();

    status_t initAndLoadAppPreferences();

    status_t setSensorFeeds();

    status_t MQTTConnect();

    status_t updateConfigFromMQTT();

    void errorHandler();

    void goToSleep();

    status_t updateConfigValues();

    status_t updateWaterThresholdMqtt();

    ConfigValue _soil_moisture_water_threshold_percent;

    Sensors _sensors;

    /**
     * MQTT
     */
    WiFiClientSecure _client;

    Adafruit_MQTT_Client _mqtt;

    Adafruit_MQTT_Publish _soc_feed;
    Adafruit_MQTT_Publish _cell_voltage_feed;
    Adafruit_MQTT_Publish _co2_ppm_feed;
    Adafruit_MQTT_Publish _air_temp_feed;
    Adafruit_MQTT_Publish _air_humidity_feed;
    Adafruit_MQTT_Publish _soil_temperature_feed;
    Adafruit_MQTT_Publish _soil_moisture_feed;
    Adafruit_MQTT_Publish _water_level_feed;
    Adafruit_MQTT_Publish _solar_panel_voltage_feed;
    Adafruit_MQTT_Publish _solar_panel_current_feed;
    Adafruit_MQTT_Publish _solar_panel_power_feed;
    Adafruit_MQTT_Publish _logging_feed;

    /*
     * Config values used to control system behaviour, always updated
     */
    MQTTConfigValue _water_pump_override_mqtt_config;

    /*
     * Config values, only updated if _should_update_config is set to ON
     */
    MQTTConfigValue _should_update_mqtt_config;
    MQTTConfigValue _water_threshold_mqtt_config;

    const char* _test_root_ca= \
         "-----BEGIN CERTIFICATE-----\n" \
         "MIIDrzCCApegAwIBAgIQCDvgVpBCRrGhdWrJWZHHSjANBgkqhkiG9w0BAQUFADBh\n" \
         "MQswCQYDVQQGEwJVUzEVMBMGA1UEChMMRGlnaUNlcnQgSW5jMRkwFwYDVQQLExB3\n" \
         "d3cuZGlnaWNlcnQuY29tMSAwHgYDVQQDExdEaWdpQ2VydCBHbG9iYWwgUm9vdCBD\n" \
         "QTAeFw0wNjExMTAwMDAwMDBaFw0zMTExMTAwMDAwMDBaMGExCzAJBgNVBAYTAlVT\n" \
         "MRUwEwYDVQQKEwxEaWdpQ2VydCBJbmMxGTAXBgNVBAsTEHd3dy5kaWdpY2VydC5j\n" \
         "b20xIDAeBgNVBAMTF0RpZ2lDZXJ0IEdsb2JhbCBSb290IENBMIIBIjANBgkqhkiG\n" \
         "9w0BAQEFAAOCAQ8AMIIBCgKCAQEA4jvhEXLeqKTTo1eqUKKPC3eQyaKl7hLOllsB\n" \
         "CSDMAZOnTjC3U/dDxGkAV53ijSLdhwZAAIEJzs4bg7/fzTtxRuLWZscFs3YnFo97\n" \
         "nh6Vfe63SKMI2tavegw5BmV/Sl0fvBf4q77uKNd0f3p4mVmFaG5cIzJLv07A6Fpt\n" \
         "43C/dxC//AH2hdmoRBBYMql1GNXRor5H4idq9Joz+EkIYIvUX7Q6hL+hqkpMfT7P\n" \
         "T19sdl6gSzeRntwi5m3OFBqOasv+zbMUZBfHWymeMr/y7vrTC0LUq7dBMtoM1O/4\n" \
         "gdW7jVg/tRvoSSiicNoxBN33shbyTApOB6jtSj1etX+jkMOvJwIDAQABo2MwYTAO\n" \
         "BgNVHQ8BAf8EBAMCAYYwDwYDVR0TAQH/BAUwAwEB/zAdBgNVHQ4EFgQUA95QNVbR\n" \
         "TLtm8KPiGxvDl7I90VUwHwYDVR0jBBgwFoAUA95QNVbRTLtm8KPiGxvDl7I90VUw\n" \
         "DQYJKoZIhvcNAQEFBQADggEBAMucN6pIExIK+t1EnE9SsPTfrgT1eXkIoyQY/Esr\n" \
         "hMAtudXH/vTBH1jLuG2cenTnmCmrEbXjcKChzUyImZOMkXDiqw8cvpOp/2PV5Adg\n" \
         "06O/nVsJ8dWO41P0jmP6P6fbtGbfYmbW0W5BjfIttep3Sp+dWOIrWcBAI+0tKIJF\n" \
         "PnlUkiaY4IBIqDfv8NZ5YBberOgOzW6sRBc4L0na4UU+Krk2U886UAb3LujEV0ls\n" \
         "YSEY1QSteDwsOoBrp+uvFRTp2InBuThs4pFsiv9kuXclVzDAGySj4dzp30d8tbQk\n" \
         "CAUw7C29C79Fv1C5qfPrmAESrciIxpg0X40KPMbp1ZWVbd4=\n" \
         "-----END CERTIFICATE-----\n";
};

#endif /* end of include guard: SYSTEMMANAGER_H_HO5YVCME */
