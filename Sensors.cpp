#include "LC709203F.h"
#include "DFRobot_CCS811.h"
#include "BME280.h"
#include "Wire.h"
#include <WiFi.h>

#include "Sensors.h"
#include "Power_Controller.h"

// Sensor setup number retries
#define NUM_SETUP_RETRIES 5

#define CCS811_BASELINE 0x3480

Sensors gSensors;

sensor_status_t Sensors::init() {
    sensor_status_t rc;

    // turn on the power
    power_controller_status_t power_rc = gPowerController.init();
    if (power_rc != POWER_CONTROLLER_OK) {
      return SENSOR_FAIL;
    }

    power_rc = gPowerController.setPowerChannel(PowerController::POWER_CHANNEL_3V3, true);
    if (rc != POWER_CONTROLLER_OK) {
      return SENSOR_FAIL;
    }

    rc = bme280_init();
    if (rc != SENSOR_OK) {
        return rc;
    }

    rc = ccs811_init();
    if (rc != SENSOR_OK) {
        return rc;
    }

    rc = gg_init();
    if (rc != SENSOR_OK) {
        return rc;
    }

    return SENSOR_OK;
}

sensor_status_t Sensors::update_all_values()
{
    sensor_status_t rc;

    rc = update_ccs811_values();
    if (rc != SENSOR_OK) {
        return rc;
    }

    rc = update_bme280_values();
    if (rc != SENSOR_OK) {
        return rc;
    }

    rc = update_gg_values();
    if (rc != SENSOR_OK) {
        return rc;
    }

    return SENSOR_OK;
}

sensor_status_t Sensors::publish_all_feeds() {
    sensor_status_t rc;

    rc = publish_co2();
    if (rc != SENSOR_OK) {
        return rc;
    }

    rc = publish_air_temp();
    if (rc != SENSOR_OK) {
        return rc;
    }

    rc = publish_soc();
    if (rc != SENSOR_OK) {
        return rc;
    }

    rc = publish_cell_voltage();
    if (rc != SENSOR_OK) {
        return rc;
    }

    return SENSOR_OK;
}

sensor_status_t Sensors::update_ccs811_values() {
    CCS811.writeBaseLine(CCS811_BASELINE);

    Serial.println("Waiting for CCS811 data to be ready");
    int num_tries;
    //for (num_tries = 0; num_tries < NUM_SETUP_RETRIES; ++num_tries) {
    while (1) {
        if (!CCS811.checkDataReady()) {
            Serial.print(".");
            delay(500);
        } else {
          break;
        }
    }
    Serial.println();

    if (num_tries == NUM_SETUP_RETRIES) {
        Serial.println("Failed to read ccs811 values, data not ready");
        return SENSOR_FAIL;
    }

    co2_ppm = CCS811.getCO2PPM();

    return SENSOR_OK;
}

sensor_status_t Sensors::publish_co2() {
    if (_co2_feed == nullptr) {
        Serial.println("CO2 Feed not set");
        return SENSOR_FAIL;
    }

    Serial.print("Sending co2 ppm val ");
    Serial.print(co2_ppm);
    Serial.print("...");
    if (!_co2_feed->publish(co2_ppm)) {
      Serial.println("Failed");
      return SENSOR_FAIL;
    } else {
      Serial.println("OK!");
      return SENSOR_OK;
    }
}

sensor_status_t Sensors::update_bme280_values() {
    air_temp_celsius = bme.getTemperature();

    return SENSOR_OK;
}

sensor_status_t Sensors::publish_air_temp() {
    if (_air_temp_feed == nullptr) {
        Serial.println("Air Temp Feed not set");
        return SENSOR_FAIL;
    }

    Serial.print("\nSending air temp val ");
    Serial.print(air_temp_celsius);
    Serial.print("...");
    if (!_air_temp_feed->publish(air_temp_celsius)) {
        Serial.println("Failed");
        return SENSOR_FAIL;
    } else {
        Serial.println("OK!");
        return SENSOR_OK;
    }
}

sensor_status_t Sensors::update_gg_values() {
    battery_soc_percent = double(gg.cellRemainingPercent10()) / 10;
    battery_voltage_mv = double(gg.cellVoltage_mV()) / 1000.0;

    return SENSOR_OK;
}

sensor_status_t Sensors::publish_soc() {
    if (_soc_feed == nullptr) {
        Serial.println("SOC Feed not set");
        return SENSOR_FAIL;
    }

    Serial.print("\nSending soc val ");
    Serial.print(battery_soc_percent);
    Serial.print("...");
    if (!_soc_feed->publish(battery_soc_percent)) {
        Serial.println("Failed");
        return SENSOR_FAIL;
    } else {
        Serial.println("OK!");
        return SENSOR_OK;
    }
}

sensor_status_t Sensors::publish_cell_voltage() {
    if (_cell_voltage_feed == nullptr) {
        Serial.println("Cell Voltage Feed not set");
        return SENSOR_FAIL;
    }

    Serial.print("\nSending cell voltage val ");
    Serial.print(battery_voltage_mv);
    Serial.print("...");
    if (!_cell_voltage_feed->publish(battery_voltage_mv)) {
        Serial.println("Failed");
        return SENSOR_FAIL;
    } else {
        Serial.println("OK!");
        return SENSOR_OK;
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

sensor_status_t Sensors::bme280_init()
{
  int setupRetries;

  bme.setI2CInterface(&Wire);
  bme.setAddr(0x76);

  bme.reset();

  Serial.println("Setting up bme280");
  for (setupRetries = 0; setupRetries < NUM_SETUP_RETRIES; setupRetries++) {
    if (bme.begin() != BME280::eStatusOK) {
      Serial.println("bme begin faild");
      Serial.println(bme_operate_status_to_string(bme.lastOperateStatus));
      delay(500);
    } else {
      break;
    }
  }

  if (setupRetries >= NUM_SETUP_RETRIES) {
    Serial.println("Gave up setting up bme280");
    return SENSOR_FAIL;
  }

  Serial.println("bme init success");

  return SENSOR_OK;
}

sensor_status_t Sensors::ccs811_init()
{
  int setupRetries;

  Serial.println("Setting up CCS811");
  for (setupRetries = 0; setupRetries < NUM_SETUP_RETRIES; setupRetries++) {
    if (CCS811.begin() != 0) {
      Serial.println("Failed to init CCS811");
      delay(500);
    } else {
      break;
    }
  }

  if (setupRetries >= NUM_SETUP_RETRIES) {
    Serial.println("Gave up setting up ccs811");
    return SENSOR_FAIL;
  }

  Serial.println("ccs811 init success");

  return SENSOR_OK;
}

sensor_status_t Sensors::gg_init()
{
  int setupRetries;

  Serial.println("Setting up gas gauge");
  for (setupRetries = 0; setupRetries < NUM_SETUP_RETRIES; setupRetries++) {
    if (!gg.begin()) {
      Serial.println("Failed to init gas gauge");
      delay(500);
    } else {
      break;
    }
  }


  if (setupRetries >= NUM_SETUP_RETRIES) {
    Serial.println("Gave up setting up ccs811");
    return SENSOR_FAIL;
  }

  gg.setCellCapacity(LC709203F_APA_2000MAH);

  gg.setAlarmVoltage(3.4);
  gg.setCellProfile( LC709203_NOM3p7_Charge4p2 ) ;

  Serial.println("ccs811 init success");

  return SENSOR_OK;
}

String Sensors::status_to_string(sensor_status_t status) {
  switch(status) {
      case SENSOR_OK:    return "Everything ok"; break;
      case SENSOR_FAIL:  return "Failure"; break;
      default: return "Unknown status"; break;
  }
}

sensor_status_t Sensors::set_soc_feed(Adafruit_MQTT_Publish *soc_feed) {
    //Serial.print("Setting soc feed: ");
    //Serial.println((uint32_t)soc_feed);

    _soc_feed = soc_feed;

    return SENSOR_OK;
}

sensor_status_t Sensors::set_cell_voltage_feed(Adafruit_MQTT_Publish *cell_voltage_feed) {
    //Serial.print("Setting cell voltage feed: ");
    //Serial.println((uint32_t)cell_voltage_feed);

    _cell_voltage_feed = cell_voltage_feed;

    return SENSOR_OK;
}

sensor_status_t Sensors::set_co2_feed(Adafruit_MQTT_Publish *co2_feed) {
    //Serial.print("Setting co2 feed: ");
    //Serial.println((uint32_t)co2_feed);

    _co2_feed = co2_feed;

    return SENSOR_OK;
}

sensor_status_t Sensors::set_air_temp_feed(Adafruit_MQTT_Publish *air_temp_feed) {
    //Serial.print("Setting air temp feed: ");
    //Serial.println((uint32_t)air_temp_feed);

    _air_temp_feed = air_temp_feed;

    return SENSOR_OK;
}
