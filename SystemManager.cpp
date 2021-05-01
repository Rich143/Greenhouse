#include "SystemManager.h"
#include "config.h"
#include "Secrets.h"
#include "Status.h"
#include "Logger.h"

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
    _logging_feed(&_mqtt, AIO_USERNAME "/feeds/greenhouse-log"),
    _pump_control_override_feed(&_mqtt, AIO_USERNAME "/feeds/pump-control-override"),
    _pump_control_override_feed_get(&_mqtt, AIO_USERNAME "/feeds/pump-control-override/get")
{}

void SystemManager::goToSleep()
{
  esp_sleep_enable_timer_wakeup(TIME_TO_SLEEP * uS_TO_S_FACTOR);
  esp_deep_sleep_start();
}

// Error handler sleeps to restart us
void SystemManager::errorHandler() {
    goToSleep();
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

    WiFi.localIP();
    LOG_INFO("WiFi connected, IP address: " + WiFi.localIP().toString());

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

    rc = gSensors.set_soc_feed(&_soc_feed);
    if (rc != STATUS_OK) {
        return rc;
    }
    rc = gSensors.set_cell_voltage_feed(&_cell_voltage_feed);
    if (rc != STATUS_OK) {
        return rc;
    }
    rc = gSensors.set_co2_feed(&_co2_ppm_feed);
    if (rc != STATUS_OK) {
        return rc;
    }
    rc = gSensors.set_air_temp_feed(&_air_temp_feed);
    if (rc != STATUS_OK) {
        return rc;
    }

    rc = gSensors.set_air_humidity_feed(&_air_humidity_feed);
    if (rc != STATUS_OK) {
        return rc;
    }

    rc = gSensors.set_soil_temp_feed(&_soil_temperature_feed);
    if (rc != STATUS_OK) {
        return rc;
    }

    rc = gSensors.set_soil_moisture_feed(&_soil_moisture_feed);
    if (rc != STATUS_OK) {
        return rc;
    }

    rc = gSensors.set_water_level_feed(&_water_level_feed);
    if (rc != STATUS_OK) {
        return rc;
    }

    rc = gSensors.set_solar_panel_voltage_feed(&_solar_panel_voltage_feed);
    if (rc != STATUS_OK) {
        return rc;
    }

    rc = gSensors.set_solar_panel_current_feed(&_solar_panel_current_feed);
    if (rc != STATUS_OK) {
        return rc;
    }

    rc = gSensors.set_solar_panel_power_feed(&_solar_panel_power_feed);
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

void SystemManager::init()
{
    status_t rc;

    Serial.begin(115200);
    delay(10);

    LOG_INFO("Starting up...");

    rc = initWifiMQTT();
    if (rc != STATUS_OK) {
        LOG_ERROR("Error initing sensors: " + status_to_string(rc));
        errorHandler();
    }

    // MQTT is connected, we can start logging over MQTT
    gLogger.enableMqttLogging(&_logging_feed);

    LOG_INFO("Initializing sensors");
    rc = gSensors.init();
    if (rc != STATUS_OK) {
        LOG_ERROR("Error initing sensors: " + status_to_string(rc));
        errorHandler();
    }

    rc = setSensorFeeds();
    if (rc != STATUS_OK) {
        LOG_ERROR("Error setting sensor MQTT feeds" + status_to_string(rc));
        errorHandler();
    }
}

void SystemManager::run()
{
    MQTTConnect();

    LOG_INFO("Reading from sensors");
    status_t rc = gSensors.update_all_values();
    if (rc != STATUS_OK) {
        LOG_ERROR("Error reading sensors: " + status_to_string(rc));
        errorHandler();
    }

    LOG_INFO("Publishing sensor data");
    rc = gSensors.publish_all_feeds();
    if (rc != STATUS_OK) {
        LOG_ERROR("Error publishing sensor data: " + status_to_string(rc));
        errorHandler();
    }

    LOG_INFO("Going to sleep for (seconds): " + String(TIME_TO_SLEEP));
    goToSleep();
}

bool SystemManager::shouldWater()
{
    if (_water_pump_override) {
        return true;
    }


    return false;
}
