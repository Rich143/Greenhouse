#include "SystemManager.h"
#include "config.h"
#include "Secrets.h"
#include "Status.h"
#include "Logger.h"
#include "AppPreferences.h"
#include "GreenhouseTelnet.h"

/************************* Config Constants *********************************/

/************************* Deep Sleep *********************************/
#define uS_TO_S_FACTOR 1000000  /* Conversion factor for micro seconds to seconds */
#define TIME_TO_SLEEP  120        /* Time ESP32 will go to sleep (in seconds) */

/************************* Other Constants *********************************/
#define NUM_SETUP_RETRIES 5

#define SOIL_MOISTURE_WATER_THRESHOLD_PERCENT_INVALID (-1)
#define DEFAULT_SOIL_MOISTURE_WATER_THRESHOLD_PERCENT SOIL_MOISTURE_WATER_THRESHOLD_PERCENT_INVALID

#define WATER_TIME_MS_INVALID (-1)
#define DEFAULT_WATER_TIME_MS WATER_TIME_MS_INVALID

SystemManager::SystemManager() :
    _mqtt(&_client, AIO_SERVER, AIO_SERVERPORT, AIO_USERNAME, AIO_KEY),
    _soc_feed(&_mqtt, AIO_USERNAME "/feeds/battery-soc"),
    _cell_voltage_feed(&_mqtt, AIO_USERNAME "/feeds/battery-cell-voltage"),
    _co2_ppm_feed(&_mqtt, AIO_USERNAME "/feeds/co2"),
    _air_temp_feed(&_mqtt, AIO_USERNAME "/feeds/air-temperature"),
    _air_humidity_feed(&_mqtt, AIO_USERNAME "/feeds/air-humidity"),
    _soil_temperature_feed(&_mqtt, AIO_USERNAME "/feeds/soil-temperature"),
    _soil_moisture_feed(&_mqtt, AIO_USERNAME "/feeds/soil-moisture"),
    _water_level_feed(&_mqtt, AIO_USERNAME "/feeds/water-level"),
    _solar_panel_voltage_feed(&_mqtt, AIO_USERNAME "/feeds/solar-panel-voltage"),
    _solar_panel_current_feed(&_mqtt, AIO_USERNAME "/feeds/solar-panel-current"),
    _solar_panel_power_feed(&_mqtt, AIO_USERNAME "/feeds/solar-panel-power"),
    _watering_feed(&_mqtt, AIO_USERNAME "/feeds/watering"),
    _local_ip_feed(&_mqtt, AIO_USERNAME "/feeds/local-ip"),
    _logging_feed(&_mqtt, AIO_USERNAME "/feeds/greenhouse-log"),
    _water_pump_override_mqtt_config(&_mqtt, AIO_USERNAME "/feeds/pump-control-override",
                         AIO_USERNAME "/feeds/pump-control-override/get"),
    _enable_telnet(&_mqtt, AIO_USERNAME "/feeds/enable-telnet",
                         AIO_USERNAME "/feeds/enable-telnet/get"),
    _should_update_mqtt_config(&_mqtt, AIO_USERNAME "/feeds/update-config",
                          AIO_USERNAME "/feeds/update-config/get"),
    _water_threshold_mqtt_config(&_mqtt, AIO_USERNAME "/feeds/water-threshold", 
                                           AIO_USERNAME "/feeds/water-threshold/get"),
    _water_time_mqtt_config(&_mqtt, AIO_USERNAME "/feeds/water-time", 
                                           AIO_USERNAME "/feeds/water-time/get")
{ }

void SystemManager::goToSleep()
{
    LOG_INFO("Going to sleep for (seconds): " + String(TIME_TO_SLEEP));
    esp_sleep_enable_timer_wakeup(TIME_TO_SLEEP * uS_TO_S_FACTOR);
    esp_deep_sleep_start();
}

void SystemManager::lightSleep(uint32_t seconds)
{
    delay(100);

    esp_sleep_enable_timer_wakeup(seconds * uS_TO_S_FACTOR);
    esp_light_sleep_start();
}


// Error handler sleeps to restart us
void SystemManager::errorHandler() {
    goToSleep();
}

long SystemManager::getAverageRSSI(int32_t numSamples)
{
    long averageRSSI = 0;
    for (int i = 0; i < numSamples; ++i) {
        averageRSSI += WiFi.RSSI();
        delay(20);
    }

    averageRSSI /= numSamples;

    return averageRSSI;
}

status_t SystemManager::initWifiMQTT()
{
    // Connect to WiFi access point.
    LOG_INFO("Connecting to " + String(WLAN_SSID));

    WiFi.begin(WLAN_SSID, WLAN_PASS);
    for (int i = 0; i < NUM_SETUP_RETRIES; i++) {
        if (WiFi.status() != WL_CONNECTED) {
            Serial.print(".");
            delay(500);
        } else {
            break;
        }
    }
    Serial.println();

    LOG_INFO("WiFi connected, IP address: " + WiFi.localIP().toString() + " RSSI " + String(getAverageRSSI(5)) + " dBm");

    _client.setCACert(_test_root_ca);

    status_t rc = MQTTConnect();
    if (rc != STATUS_OK) {
        return rc;
    }

    return STATUS_OK;
}

status_t SystemManager::setSensorFeeds()
{
    status_t rc;

    LOG_INFO("Setting sensor mqtt feeds");

    rc = _sensors.set_soc_feed(&_soc_feed);
    if (rc != STATUS_OK) {
        return rc;
    }
    rc = _sensors.set_cell_voltage_feed(&_cell_voltage_feed);
    if (rc != STATUS_OK) {
        return rc;
    }
    rc = _sensors.set_co2_feed(&_co2_ppm_feed);
    if (rc != STATUS_OK) {
        return rc;
    }
    rc = _sensors.set_air_temp_feed(&_air_temp_feed);
    if (rc != STATUS_OK) {
        return rc;
    }

    rc = _sensors.set_air_humidity_feed(&_air_humidity_feed);
    if (rc != STATUS_OK) {
        return rc;
    }

    rc = _sensors.set_soil_temp_feed(&_soil_temperature_feed);
    if (rc != STATUS_OK) {
        return rc;
    }

    rc = _sensors.set_soil_moisture_feed(&_soil_moisture_feed);
    if (rc != STATUS_OK) {
        return rc;
    }

    rc = _sensors.set_water_level_feed(&_water_level_feed);
    if (rc != STATUS_OK) {
        return rc;
    }

    rc = _sensors.set_solar_panel_voltage_feed(&_solar_panel_voltage_feed);
    if (rc != STATUS_OK) {
        return rc;
    }

    rc = _sensors.set_solar_panel_current_feed(&_solar_panel_current_feed);
    if (rc != STATUS_OK) {
        return rc;
    }

    rc = _sensors.set_solar_panel_power_feed(&_solar_panel_power_feed);
    if (rc != STATUS_OK) {
        return rc;
    }

    return STATUS_OK;
}

// Function to connect and reconnect as necessary to the MQTT server.
status_t SystemManager::MQTTConnect() {
  int8_t ret;

  // Stop if already connected.
  if (_mqtt.connected()) {
    return STATUS_OK;
  }

  LOG_INFO("Connecting to MQTT... ");

  uint8_t retries = 3;
  while ((ret = _mqtt.connect()) != 0) { // connect will return 0 for connected
    Serial.println(_mqtt.connectErrorString(ret));
    LOG_WARN(reinterpret_cast<const char *>(_mqtt.connectErrorString(ret)));

    LOG_INFO("Retrying MQTT connection in 5 seconds...");
    _mqtt.disconnect();
    delay(5000);  // wait 5 seconds
    retries--;
    if (retries == 0) {
      LOG_ERROR("Failed to connect to MQTT");
      return STATUS_FAIL;
    }
  }
  LOG_INFO("MQTT Connected!");

  return STATUS_OK;
}

status_t SystemManager::initAndLoadAppPreferences()
{
    status_t rc;

    if (!gAppPreferences.begin("AppPreferences", false)) {
        return STATUS_FAIL;
    }

    rc = _soil_moisture_water_threshold_percent.initAndLoad(
        "waterThresh", DEFAULT_SOIL_MOISTURE_WATER_THRESHOLD_PERCENT);
    if (rc != STATUS_OK) {
        LOG_ERROR("Failed to load water threshold config value");
        return rc;
    }

    rc = _water_time_seconds.initAndLoad(
        "waterTime", DEFAULT_WATER_TIME_MS);
    if (rc != STATUS_OK) {
        LOG_ERROR("Failed to load water time config value");
        return rc;
    }

    return STATUS_OK;
}

status_t SystemManager::initMQTTConfigValues()
{
    status_t rc;

    rc = _water_pump_override_mqtt_config.init();
    if (rc != STATUS_OK) {
        return rc;
    }

    rc = _enable_telnet.init();
    if (rc != STATUS_OK) {
        return rc;
    }

    rc = _should_update_mqtt_config.init();
    if (rc != STATUS_OK) {
        return rc;
    }

    rc = _water_threshold_mqtt_config.init();
    if (rc != STATUS_OK) {
        return rc;
    }

    rc = _water_time_mqtt_config.init();
    if (rc != STATUS_OK) {
        return rc;
    }

    return STATUS_OK;
}

void SystemManager::init()
{
    status_t rc;

    Serial.begin(115200);
    delay(10);

    LOG_INFO("Starting up...");

    rc = initAndLoadAppPreferences();
    if (rc != STATUS_OK) {
        LOG_ERROR("Error initing and loading app preferences: " + status_to_string(rc));
        errorHandler();
    }

    rc = initMQTTConfigValues();
    if (rc != STATUS_OK) {
        LOG_ERROR("Error initing MQTT Config values " + status_to_string(rc));
        errorHandler();
    }

    rc = initWifiMQTT();
    if (rc != STATUS_OK) {
        LOG_ERROR("Error initing sensors: " + status_to_string(rc));
        errorHandler();
    }

    // MQTT is connected, we can start logging over MQTT
    gLogger.enableMqttLogging(&_logging_feed);

    LOG_INFO("Initializing sensors");
    rc = _sensors.init();
    if (rc != STATUS_OK) {
        LOG_ERROR("Error initing sensors: " + status_to_string(rc));
        errorHandler();
    }

    rc = setSensorFeeds();
    if (rc != STATUS_OK) {
        LOG_ERROR("Error setting sensor MQTT feeds" + status_to_string(rc));
        errorHandler();
    }

    rc = waterPump.init();
    if (rc != STATUS_OK) {
        LOG_ERROR("Failed to init water pump");
        errorHandler();
    }
}

status_t SystemManager::updateAndPublishSensors() {
    LOG_INFO("Reading from sensors");
    status_t rc = _sensors.update_all_values();
    if (rc != STATUS_OK) {
        LOG_ERROR("Error reading sensors: " + status_to_string(rc));
        return rc;
    }

    LOG_INFO("Publishing sensor data");
    rc = _sensors.publish_all_feeds();
    if (rc != STATUS_OK) {
        LOG_ERROR("Error publishing sensor data: " + status_to_string(rc));
        return rc;
    }

    return STATUS_OK;
}

status_t SystemManager::updateConfigValueFromMQTT(ConfigValue& configValue,
                                                  MQTTConfigValue &mqttValue)
{
    status_t rc;
    rc = mqttValue.updateValue();
    if (rc != STATUS_OK) {
        return rc;
    }
   
    rc = configValue.updateValue(
        mqttValue.getValueDouble());
    if (rc != STATUS_OK) {
        return rc;
    }

    return STATUS_OK;
}

status_t SystemManager::updateConfigValues()
{
    status_t rc;

    rc = _should_update_mqtt_config.updateValue();
    if (rc != STATUS_OK) {
        LOG_ERROR("Failed to update should update config mqtt value");
        return rc;
    }

    if (_should_update_mqtt_config.getValueOnOff()) {
        LOG_INFO("Updating Config values");

        rc = updateConfigValueFromMQTT(_soil_moisture_water_threshold_percent,
                                       _water_threshold_mqtt_config);
        if (rc != STATUS_OK) {
            return rc;
        }

        rc = updateConfigValueFromMQTT(_water_time_seconds,
                                       _water_time_mqtt_config);
        if (rc != STATUS_OK) {
            return rc;
        }

        LOG_ALWAYS("New config: water threshold " +
                   String(_soil_moisture_water_threshold_percent.getValue()) +
                   " water time " + String(_water_time_seconds.getValue()));
    }

    return STATUS_OK;
}

status_t SystemManager::waterPlants()
{
    if (_water_time_seconds.getValue() == WATER_TIME_MS_INVALID) {
        LOG_ERROR("Water Time not configured, skipping watering");
        return STATUS_FAIL;
    }

    LOG_DEBUG("Start watering");
    _watering_feed.publish(1);
    waterPump.turnOn();

    delay(_water_time_seconds.getValue() * 1000);

    waterPump.turnOff();
    _watering_feed.publish(0);
    LOG_DEBUG("Done watering");
}

void SystemManager::checkAndStartTelnet()
{
    status_t rc;

    LOG_INFO("Checking if should enable telnet");
    rc = _enable_telnet.updateValue();
    if (rc != STATUS_OK) {
        LOG_ERROR("Failed to get enable telnet mqtt value");
        return;
    }

    if (_enable_telnet.getValueOnOff()) {
        if (!_local_ip_feed.publish(WiFi.localIP().toString().c_str())) {
            LOG_ERROR("Failed to publish telnet ip");
            return;
        }

        rc = gGreenhouseTelnet.start(this);
        if (rc != STATUS_OK) {
            LOG_ERROR("Failed to start telnet");
            return;
        }

        while (1) {
            gGreenhouseTelnet.run();
        }
    } else {
        LOG_DEBUG("Telnet not enabled");
    }
}

void SystemManager::run()
{
    status_t rc;

    MQTTConnect();

    checkAndStartTelnet();

    rc = updateAndPublishSensors();
    if (rc != STATUS_OK) {
        errorHandler();
    }

    rc = updateConfigValues();
    if (rc != STATUS_OK) {
        // This is not a serious error, so continue with other operations
    }

    if (shouldWater()) {
        LOG_DEBUG("Watering the plants");
        waterPlants();
    } else {
        LOG_DEBUG("Not watering the plants");
    }

    goToSleep();
}

bool SystemManager::shouldWater()
{
    status_t rc;
    rc = _water_pump_override_mqtt_config.updateValue();
    if (rc != STATUS_OK) {
        LOG_ERROR("Failed to update water pump override mqtt value");
    }

    if (_water_pump_override_mqtt_config.getValueOnOff()) {
        LOG_INFO("Water override is set, watering");
        return true;
    }

    if (_soil_moisture_water_threshold_percent.getValue()
        == SOIL_MOISTURE_WATER_THRESHOLD_PERCENT_INVALID)
    {
        LOG_INFO("Soil moisture water threshold not configured, skipping watering");
        return false;
    }

    double soil_moisture_percent = _sensors.getSoilMoisturePercentage();

    if (soil_moisture_percent <
        _soil_moisture_water_threshold_percent.getValue())
    {
        return true;
    } else {
        return false;
    }
}
