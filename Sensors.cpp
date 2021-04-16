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

#define CCS811_BASELINE 0x3480

Sensors gSensors;

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

    rc = update_thermistor_values();
    if (rc != STATUS_OK) {
        return rc;
    }

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

    return STATUS_OK;
}

status_t Sensors::update_thermistor_values() {
    soil_temperature_celsius = thermistor.readTemperature();

    return STATUS_OK;
}

status_t Sensors::update_ccs811_values() {
    CCS811.writeBaseLine(CCS811_BASELINE);

    LOG_INFO("Waiting for CCS811 data to be ready");
    int num_tries;
    //for (num_tries = 0; num_tries < NUM_SETUP_RETRIES; ++num_tries) {
    while (1) {
        if (!CCS811.checkDataReady()) {
            delay(500);
        } else {
          break;
        }
    }

    if (num_tries == NUM_SETUP_RETRIES) {
        LOG_ERROR("Failed to read ccs811 values, data not ready");
        return STATUS_FAIL;
    }

    co2_ppm = CCS811.getCO2PPM();

    return STATUS_OK;
}

status_t Sensors::publish_co2() {
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
}

status_t Sensors::update_bme280_values() {
    air_temp_celsius = bme.getTemperature();

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

// show last sensor operate status
String Sensors::bme_operate_status_to_string(BME280::eStatus_t eStatus)
{
  switch(eStatus) {
      case BME280::eStatusOK:    return "everything ok"; break;
      case BME280::eStatusErr:   return "unknow error"; break;
      case BME280::eStatusErrDeviceNotDetected:    return "device not detected"; break;
      case BME280::eStatusErrParameter:    return "parameter error"; break;
      default: return "unknown status"; break;
  }
}

status_t Sensors::bme280_init()
{
  int setupRetries;

  bme.setI2CInterface(&Wire);
  bme.setAddr(0x76);

  bme.reset();

  LOG_INFO("Setting up bme280");
  for (setupRetries = 0; setupRetries < NUM_SETUP_RETRIES; setupRetries++) {
    if (bme.begin() != BME280::eStatusOK) {
      LOG_WARN("bme begin failed: " + bme_operate_status_to_string(bme.lastOperateStatus));
      delay(500);
    } else {
      break;
    }
  }

  if (setupRetries >= NUM_SETUP_RETRIES) {
    LOG_ERROR("Gave up setting up bme280");
    return STATUS_FAIL;
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

String Sensors::status_to_string(status_t status) {
  switch(status) {
      case STATUS_OK:    return "Everything ok"; break;
      case STATUS_INVALID_PARAMS: return "Invalid Params"; break;
      case STATUS_FAIL:  return "Failure"; break;
      default: return "Unknown status"; break;
  }
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

status_t Sensors::set_soil_temp_feed(Adafruit_MQTT_Publish *soil_temperature_feed) {
    _soil_temp_feed = soil_temperature_feed;

    return STATUS_OK;
}
