#include "LC709203F.h"
#include "DFRobot_CCS811.h"
#include "BME280.h"
#include "Wire.h"
#include <WiFi.h>

#include "Sensors.h"
#include "Power_Controller.h"
#include "Status.h"
#include "Logger.h"

// Sensor setup number retries
#define NUM_SETUP_RETRIES 5

//#define ENABLE_CCS811

// CCS811 has a preheat time of up to 15 seconds
#define CCS811_PREHEAT_TIME_MS (15*1000)
#define CCS811_WAIT_TIME_MS (500)
#define CCS811_MAX_WAITS (CCS811_PREHEAT_TIME_MS / CCS811_WAIT_TIME_MS)

#define CCS811_BASELINE 0xA477

#define WATERLEVEL_INVALID_MEASUREMENT (-1)

status_t Sensors::init() {
    status_t rc;

    // turn on the power
    rc = gPowerController.init();
    if (rc != STATUS_OK) {
      return STATUS_FAIL;
    }

    rc = gPowerController.setPowerChannel(PowerController::POWER_CHANNEL_3V3, true);
    if (rc != STATUS_OK) {
      return rc;
    }

    rc = bme280_init();
    if (rc != STATUS_OK) {
        return rc;
    }

    rc = ccs811_init();
    if (rc != STATUS_OK) {
        return rc;
    }

    rc = gg_init();
    if (rc != STATUS_OK) {
        return rc;
    }

    rc = ina219_init();
    if (rc != STATUS_OK) {
        return rc;
    }

    return STATUS_OK;
}

status_t Sensors::update_all_values()
{
    status_t rc;

    rc = update_ccs811_values();
    if (rc != STATUS_OK) {
        return rc;
    }

    rc = update_bme280_values();
    if (rc != STATUS_OK) {
        return rc;
    }

    rc = update_gg_values();
    if (rc != STATUS_OK) {
        return rc;
    }

    rc = update_ina219_values();
    if (rc != STATUS_OK) {
        return rc;
    }

    rc = update_thermistor_values();
    if (rc != STATUS_OK) {
        return rc;
    }

    rc = update_soil_moisture_values();
    if (rc != STATUS_OK) {
        return rc;
    }

    rc = update_water_level_values();
    if (rc != STATUS_OK) {
        if (rc == STATUS_TIMEOUT) {
            // timeouts aren't an error, just log it
            LOG_WARN("Didn't get valid water level measurement");
            return STATUS_OK;
        } else {
            return rc;
        }
    }

    delay(1000);

    return STATUS_OK;
}

status_t Sensors::publish_all_feeds() {
    status_t rc;

    rc = publish_co2();
    if (rc != STATUS_OK) {
        return rc;
    }

    rc = publish_air_temp();
    if (rc != STATUS_OK) {
        return rc;
    }

    rc = publish_air_humidity();
    if (rc != STATUS_OK) {
        return rc;
    }

    rc = publish_soc();
    if (rc != STATUS_OK) {
        return rc;
    }

    rc = publish_cell_voltage();
    if (rc != STATUS_OK) {
        return rc;
    }

    rc = publish_soil_temp();
    if (rc != STATUS_OK) {
        return rc;
    }

    rc = publish_soil_moisture();
    if (rc != STATUS_OK) {
        return rc;
    }

    rc = publish_water_level();
    if (rc != STATUS_OK) {
        return rc;
    }

    rc = publish_solar_panel_voltage();
    if (rc != STATUS_OK) {
        return rc;
    }

    rc = publish_solar_panel_current();
    if (rc != STATUS_OK) {
        return rc;
    }

    rc = publish_solar_panel_power();
    if (rc != STATUS_OK) {
        return rc;
    }

    return STATUS_OK;
}

status_t Sensors::update_thermistor_values() {
    soil_temperature_celsius = thermistor.readTemperature();

    return STATUS_OK;
}

status_t Sensors::update_ccs811_values() {
#ifdef ENABLE_CCS811
    CCS811.writeBaseLine(CCS811_BASELINE);

    LOG_INFO("Waiting for ccs811 to warm up");
    delay(15000);

    LOG_INFO("Waiting for CCS811 data to be ready");
    int num_tries;
    //for (num_tries = 0; num_tries < CCS811_MAX_WAITS; ++num_tries) {
    while (1) {
        if (!CCS811.checkDataReady()) {
            delay(CCS811_WAIT_TIME_MS);
        } else {
          break;
        }
    }

    if (num_tries == NUM_SETUP_RETRIES) {
        LOG_ERROR("Failed to read ccs811 values, data not ready");
        return STATUS_FAIL;
    }

    co2_ppm = CCS811.getCO2PPM();

#endif

    return STATUS_OK;
}

status_t Sensors::publish_co2() {
#ifdef ENABLE_CCS811
    if (_co2_feed == nullptr) {
        LOG_ERROR("CO2 Feed not set");
        return STATUS_FAIL;
    }

    LOG_INFO("Sending co2 ppm val: " + String(co2_ppm));
    if (!_co2_feed->publish(co2_ppm)) {
      LOG_ERROR("Failed to publish co2 ppm value");
      return STATUS_FAIL;
    } else {
      LOG_INFO("OK!");
      return STATUS_OK;
    }
#else
    return STATUS_OK;
#endif
}

status_t Sensors::update_bme280_values() {
   BME280::TempUnit tempUnit(BME280::TempUnit_Celsius);
   BME280::PresUnit presUnit(BME280::PresUnit_Pa);
   float pres, temp, hum;

   bme.read(pres, temp, hum, tempUnit, presUnit);

   air_temp_celsius = temp;
   air_humidity_percent = hum;

  return STATUS_OK;
}

status_t Sensors::publish_air_temp() {
    if (_air_temp_feed == nullptr) {
        LOG_ERROR("Air Temp Feed not set");
        return STATUS_FAIL;
    }

    LOG_INFO("\nSending air temp val: " + String(air_temp_celsius));
    if (!_air_temp_feed->publish(air_temp_celsius)) {
        LOG_ERROR("Failed to publish air temp value");
        return STATUS_FAIL;
    } else {
        LOG_INFO("OK!");
        return STATUS_OK;
    }
}

status_t Sensors::publish_air_humidity() {
    if (_air_humidity_feed == nullptr) {
        LOG_ERROR("Air Humidity Feed not set");
        return STATUS_FAIL;
    }

    LOG_INFO("\nSending air humidity val: " + String(air_humidity_percent));
    if (!_air_humidity_feed->publish(air_humidity_percent)) {
        LOG_ERROR("Failed to publish air humidity value");
        return STATUS_FAIL;
    } else {
        LOG_INFO("OK!");
        return STATUS_OK;
    }
}

status_t Sensors::publish_soil_temp() {
    if (_soil_temp_feed == nullptr) {
        LOG_ERROR("Soil Temp Feed not set");
        return STATUS_FAIL;
    }

    LOG_INFO("\nSending soil temp val: " + String(soil_temperature_celsius));
    if (!_soil_temp_feed->publish(soil_temperature_celsius)) {
        LOG_ERROR("Failed to publish soil temp value");
        return STATUS_FAIL;
    } else {
        LOG_INFO("OK!");
        return STATUS_OK;
    }
}

status_t Sensors::update_gg_values() {
    battery_soc_percent = double(gg.cellRemainingPercent10()) / 10;
    battery_voltage_mv = double(gg.cellVoltage_mV()) / 1000.0;

    return STATUS_OK;
}

status_t Sensors::publish_soc() {
    if (_soc_feed == nullptr) {
        LOG_ERROR("SOC Feed not set");
        return STATUS_FAIL;
    }

    LOG_INFO("Sending soc val: " + String(battery_soc_percent));
    if (!_soc_feed->publish(battery_soc_percent)) {
        LOG_ERROR("Failed to publish soc value");
        return STATUS_FAIL;
    } else {
        LOG_INFO("OK!");
        return STATUS_OK;
    }
}

status_t Sensors::publish_cell_voltage() {
    if (_cell_voltage_feed == nullptr) {
        LOG_ERROR("Cell Voltage Feed not set");
        return STATUS_FAIL;
    }

    LOG_INFO("\nSending cell voltage val: " + String(battery_voltage_mv));
    if (!_cell_voltage_feed->publish(battery_voltage_mv)) {
        LOG_ERROR("Failed to publish cell voltage feed");
        return STATUS_FAIL;
    } else {
        LOG_INFO("OK!");
        return STATUS_OK;
    }
}

status_t Sensors::publish_solar_panel_voltage() {
    if (_solar_panel_voltage_feed == nullptr) {
        LOG_ERROR("Solar Panel Voltage Feed not set");
        return STATUS_FAIL;
    }

    LOG_INFO("\nSending solar panel voltage val: " + String(solar_panel_voltage_V));
    if (!_solar_panel_voltage_feed->publish(solar_panel_voltage_V)) {
        LOG_ERROR("Failed to publish solar panel voltage feed");
        return STATUS_FAIL;
    } else {
        LOG_INFO("OK!");
        return STATUS_OK;
    }
}

status_t Sensors::publish_solar_panel_current() {
    if (_solar_panel_current_feed == nullptr) {
        LOG_ERROR("Solar Panel Current Feed not set");
        return STATUS_FAIL;
    }

    LOG_INFO("\nSending solar panel current val: " + String(solar_panel_current_mA));
    if (!_solar_panel_current_feed->publish(solar_panel_current_mA)) {
        LOG_ERROR("Failed to publish solar panel current feed");
        return STATUS_FAIL;
    } else {
        LOG_INFO("OK!");
        return STATUS_OK;
    }
}

status_t Sensors::publish_solar_panel_power() {
    if (_solar_panel_power_feed == nullptr) {
        LOG_ERROR("Solar Panel Power Feed not set");
        return STATUS_FAIL;
    }

    LOG_INFO("\nSending solar panel power val: " + String(solar_panel_power_mW));
    if (!_solar_panel_power_feed->publish(solar_panel_power_mW)) {
        LOG_ERROR("Failed to publish solar panel power feed");
        return STATUS_FAIL;
    } else {
        LOG_INFO("OK!");
        return STATUS_OK;
    }
}

status_t Sensors::publish_soil_moisture() {
    if (_soil_moisture_feed == nullptr) {
        LOG_ERROR("Soil Moisture Feed not set");
        return STATUS_FAIL;
    }

    LOG_INFO("\nSending soil moisture val: " + String(soil_moisture_percent));
    if (!_soil_moisture_feed->publish(soil_moisture_percent)) {
        LOG_ERROR("Failed to publish soil moisture feed");
        return STATUS_FAIL;
    } else {
        LOG_INFO("OK!");
        return STATUS_OK;
    }
}

status_t Sensors::publish_water_level() {
    if (_water_level_feed == nullptr) {
        LOG_ERROR("Water Level Feed not set");
        return STATUS_FAIL;
    }

    if (water_level_percent == WATERLEVEL_INVALID_MEASUREMENT) {
        // Skip publishing invalid value
        // Logging of this error happens in update function
        return STATUS_OK;
    }

    LOG_INFO("\nSending water level val: " + String(water_level_percent));
    if (!_water_level_feed->publish(water_level_percent)) {
        LOG_ERROR("Failed to publish water level feed");
        return STATUS_FAIL;
    } else {
        LOG_INFO("OK!");
        return STATUS_OK;
    }
}

status_t Sensors::bme280_init()
{
  int setupRetries;

  Wire.begin();

  LOG_INFO("Setting up bme280");
  for (setupRetries = 0; setupRetries < NUM_SETUP_RETRIES; setupRetries++) {
    if (!bme.begin()) {
      LOG_WARN("bme begin failed");
      delay(500);
    } else {
      break;
    }
  }

  if (setupRetries >= NUM_SETUP_RETRIES) {
    LOG_ERROR("Gave up setting up bme280");
    return STATUS_FAIL;
  }

  switch(bme.chipModel())
  {
     case BME280::ChipModel_BME280:
       LOG_INFO("Found BME280 sensor! Success.");
       break;
     case BME280::ChipModel_BMP280:
       LOG_INFO("Found BMP280 sensor! No Humidity available.");
       break;
     default:
       LOG_INFO("Found UNKNOWN sensor! Error!");
  }

  LOG_INFO("bme init success");

  return STATUS_OK;
}

status_t Sensors::ccs811_init()
{
  int setupRetries;

  LOG_INFO("Setting up CCS811");
  for (setupRetries = 0; setupRetries < NUM_SETUP_RETRIES; setupRetries++) {
    if (CCS811.begin() != 0) {
      LOG_WARN("Failed to init CCS811");
      delay(500);
    } else {
      break;
    }
  }

  if (setupRetries >= NUM_SETUP_RETRIES) {
    LOG_ERROR("Gave up setting up ccs811");
    return STATUS_FAIL;
  }

  LOG_INFO("ccs811 init success");

  return STATUS_OK;
}

status_t Sensors::gg_init()
{
  int setupRetries;

  LOG_INFO("Setting up gas gauge");
  for (setupRetries = 0; setupRetries < NUM_SETUP_RETRIES; setupRetries++) {
    if (!gg.begin()) {
      LOG_WARN("Failed to init gas gauge");
      delay(500);
    } else {
      break;
    }
  }


  if (setupRetries >= NUM_SETUP_RETRIES) {
    LOG_ERROR("Gave up setting up gas gauge");
    return STATUS_FAIL;
  }

  gg.setCellCapacity(LC709203F_APA_2000MAH);

  gg.setAlarmVoltage(3.4);
  gg.setCellProfile( LC709203_NOM3p7_Charge4p2 ) ;

  LOG_INFO("ccs811 init success");

  return STATUS_OK;
}

status_t Sensors::ina219_init() {
  int setupRetries;

  LOG_INFO("Setting up INA219");
  for (setupRetries = 0; setupRetries < NUM_SETUP_RETRIES; setupRetries++) {
    if (!ina219.begin()) {
      LOG_WARN("Failed to init INA219");
      delay(500);
    } else {
      break;
    }
  }

  if (setupRetries >= NUM_SETUP_RETRIES) {
    LOG_ERROR("Gave up setting up INA219");
    return STATUS_FAIL;
  }

  LOG_INFO("INA219 init success");

  return STATUS_OK;
}

status_t Sensors::update_ina219_values() {
  solar_panel_voltage_V = ina219.getBusVoltage_V();
  solar_panel_current_mA = ina219.getCurrent_mA();
  solar_panel_power_mW = ina219.getPower_mW();

  return STATUS_OK;
}

status_t Sensors::update_soil_moisture_values() {
    soil_moisture_percent = soilMoisture.soilMoisturePercent();

    return STATUS_OK;
}

status_t Sensors::update_water_level_values() {
    status_t rc =  waterLevel.getWaterLevelPercent(&water_level_percent);

    // If we get a timeout, mark the value as invalid so we don't publish it
    if (rc == STATUS_TIMEOUT) {
        water_level_percent = WATERLEVEL_INVALID_MEASUREMENT;
    }

    return rc;
}

status_t Sensors::set_soc_feed(Adafruit_MQTT_Publish *soc_feed) {
    _soc_feed = soc_feed;

    return STATUS_OK;
}

status_t Sensors::set_cell_voltage_feed(Adafruit_MQTT_Publish *cell_voltage_feed) {
    _cell_voltage_feed = cell_voltage_feed;

    return STATUS_OK;
}

status_t Sensors::set_co2_feed(Adafruit_MQTT_Publish *co2_feed) {
    _co2_feed = co2_feed;

    return STATUS_OK;
}

status_t Sensors::set_air_temp_feed(Adafruit_MQTT_Publish *air_temp_feed) {
    _air_temp_feed = air_temp_feed;

    return STATUS_OK;
}

status_t Sensors::set_air_humidity_feed(Adafruit_MQTT_Publish *air_humidity_feed) {
    _air_humidity_feed = air_humidity_feed;

    return STATUS_OK;
}

status_t Sensors::set_soil_temp_feed(Adafruit_MQTT_Publish *soil_temperature_feed) {
    _soil_temp_feed = soil_temperature_feed;

    return STATUS_OK;
}

status_t Sensors::set_soil_moisture_feed(Adafruit_MQTT_Publish *soil_moisture_feed) {
    _soil_moisture_feed = soil_moisture_feed;

    return STATUS_OK;
}

status_t Sensors::set_water_level_feed(Adafruit_MQTT_Publish *water_level_feed) {
    _water_level_feed = water_level_feed;

    return STATUS_OK;
}

status_t Sensors::set_solar_panel_voltage_feed(Adafruit_MQTT_Publish *solar_panel_voltage_feed) {
  _solar_panel_voltage_feed = solar_panel_voltage_feed;

  return STATUS_OK;
}

status_t Sensors::set_solar_panel_current_feed(Adafruit_MQTT_Publish *solar_panel_current_feed) {
  _solar_panel_current_feed = solar_panel_current_feed;

  return STATUS_OK;
}

status_t Sensors::set_solar_panel_power_feed(Adafruit_MQTT_Publish *solar_panel_power_feed) {
  _solar_panel_power_feed = solar_panel_power_feed;

  return STATUS_OK;
}

double Sensors::getSoilMoisturePercentage() {
  return soil_moisture_percent;
}
