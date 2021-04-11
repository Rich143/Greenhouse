#include "LC709203F.h"
#include "DFRobot_CCS811.h"
#include "DFRobot_BME280.h"
#include "Wire.h"
#include <WiFi.h>

#include "Sensors.h"

// Sensor setup number retries
#define NUM_SETUP_RETRIES 5

#define CCS811_BASELINE 0x3480

sensor_status_t Sensors::init() {
    sensor_status_t rc;

    rc= bme280_init();
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
    for (num_tries = 0; num_tries < NUM_SETUP_RETRIES; ++num_tries) {
        if (!CCS811.checkDataReady()) {
            delay(100);
        }
    }

    if (num_tries == NUM_SETUP_RETRIES) {
        Serial.println("Failed to read ccs811 values");
        return SENSOR_FAIL;
    }

    co2_ppm = CCS811.getCO2PPM();
}

sensor_status_t Sensors::publish_co2() {
    Serial.print("Sending co2 ppm val ");
    Serial.print(co2_ppm);
    Serial.print("...");
    if (!_co2_feed.publish(co2_ppm)) {
      Serial.println("Failed");
      return SENSOR_FAIL;
    } else {
      Serial.println("OK!");
      return SENSOR_OK;
    }
}

sensor_status_t Sensors::update_bme280_values() {
    air_temp_celsius = bme.getTemperature();
}

sensor_status_t Sensors::publish_air_temp() {
    Serial.print("\nSending air temp val ");
    Serial.print(air_temp_celsius);
    Serial.print("...");
    if (!_air_temp_feed.publish(air_temp_celsius)) {
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
}

sensor_status_t Sensors::publish_soc() {
    Serial.print("\nSending soc val ");
    Serial.print(battery_soc_percent);
    Serial.print("...");
    if (!_soc_feed.publish(battery_soc_percent)) {
        Serial.println("Failed");
        return SENSOR_FAIL;
    } else {
        Serial.println("OK!");
        return SENSOR_OK;
    }
}

sensor_status_t Sensors::publish_cell_voltage() {
  Serial.print("\nSending cell voltage val ");
  Serial.print(battery_voltage_mv);
  Serial.print("...");
  if (!_cell_voltage_feed.publish(battery_voltage_mv)) {
    Serial.println("Failed");
    return SENSOR_FAIL;
  } else {
    Serial.println("OK!");
    return SENSOR_OK;
  }
}

// show last sensor operate status
std::string Sensors::bme_operate_status_to_string(BME::eStatus_t eStatus)
{
  switch(eStatus) {
      case BME::eStatusOK:    return "everything ok"; break;
      case BME::eStatusErr:   return "unknow error"; break;
      case BME::eStatusErrDeviceNotDetected:    return "device not detected"; break;
      case BME::eStatusErrParameter:    return "parameter error"; break;
      default: return "unknown status"; break;
  }
}

sensor_status_t Sensors::bme280_init()
{
  int setupRetries;

  bme.reset();

  Serial.println("Setting up bme280");
  for (setupRetries = 0; setupRetries < NUM_SETUP_RETRIES; setupRetries++) {
    if (bme.begin() != BME::eStatusOK) {
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

static std::string Sensors::status_to_string(sensor_status_t status) {
  switch(status) {
      case SENSOR_OK:    return "Everything ok"; break;
      case SENSOR_FAIL:  return "Failure"; break;
      default: return "Unknown status"; break;
  }
}

