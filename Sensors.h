#ifndef __SENSORS_H
#define __SENSORS_H

#include "Adafruit_MQTT.h"
#include "Adafruit_MQTT_Client.h"
#include "LC709203F.h"
#include "DFRobot_CCS811.h"
#include "BME280.h"
#include "Status.h"

class Sensors {
    public:

    Sensors() {}

    status_t init();

    status_t update_all_values();

    status_t set_soc_feed(Adafruit_MQTT_Publish *soc_feed);
    status_t set_cell_voltage_feed(Adafruit_MQTT_Publish *cell_voltage_feed);
    status_t set_co2_feed(Adafruit_MQTT_Publish *co2_feed);
    status_t set_air_temp_feed(Adafruit_MQTT_Publish *air_temp_feed);

    /**
      * @brief Publishes current sensor data to the MQTT feeds
      */
    status_t publish_all_feeds();

    static String status_to_string(status_t status);

    private:

    status_t bme280_init();
    status_t ccs811_init();
    status_t gg_init();

    status_t update_ccs811_values();
    status_t update_bme280_values();
    status_t update_gg_values();

    status_t publish_co2();
    status_t publish_air_temp();
    status_t publish_soc();
    status_t publish_cell_voltage();

    String bme_operate_status_to_string(BME280::eStatus_t eStatus);

    double co2_ppm;
    double air_temp_celsius;
    double battery_soc_percent;
    double battery_voltage_mv;

    Adafruit_MQTT_Publish *_soc_feed;
    Adafruit_MQTT_Publish *_cell_voltage_feed;
    Adafruit_MQTT_Publish *_co2_feed;
    Adafruit_MQTT_Publish *_air_temp_feed;

    // Gas Gauge
    LC709203F gg;

    // CO2 and VOC sensor
    DFRobot_CCS811 CCS811;

    // Weather monitor sensor
    BME280 bme;
};

// global sensors instance
extern Sensors gSensors;

#endif /* end of include guard: __SENSORS_H */
