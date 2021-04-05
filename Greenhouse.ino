/***************************************************
  Greenhouse monitor

 ****************************************************/

#include "LC709203F.h"
#include "DFRobot_CCS811.h"
#include "DFRobot_BME280.h"
#include "Wire.h"
#include <WiFi.h>
#include <WiFiClientSecure.h>
#include "Adafruit_MQTT.h"
#include "Adafruit_MQTT_Client.h"

/************************* Deep Sleep *********************************/

#define uS_TO_S_FACTOR 1000000  /* Conversion factor for micro seconds to seconds */
#define TIME_TO_SLEEP  60        /* Time ESP32 will go to sleep (in seconds) */

/************************* WiFi Access Point *********************************/

#define WLAN_SSID       "Matthews-NCF"
#define WLAN_PASS       "OurNetwork@Home"

/************************* Adafruit.io Setup *********************************/

#define AIO_SERVER      "io.adafruit.com"
#define AIO_SERVERPORT  8883                   // use 8883 for SSL, or 1883 for non secure
#define AIO_USERNAME    "Rich_M"
#define AIO_KEY         "aio_znZz20yk7U58KKP78wYzwbmVpXew"


/************************* Other Constants *********************************/

#define NUM_SETUP_RETRIES 5
#define CCS811_BASELINE 0x3480

typedef DFRobot_BME280_IIC    BME;

/************ Global State ******************/

RTC_DATA_ATTR int bootCount = 0;

WiFiClientSecure client;

const char* test_root_ca= \
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

// Setup the MQTT client class by passing in the WiFi client and MQTT server and login details.
Adafruit_MQTT_Client mqtt(&client, AIO_SERVER, AIO_SERVERPORT, AIO_USERNAME, AIO_KEY);

// Gas Gauge
LC709203F gg;

DFRobot_CCS811 CCS811;

BME   bme(&Wire, 0x76);

/****************************** Feeds ***************************************/

// Setup a feed called 'photocell' for publishing.
// Notice MQTT paths for AIO follow the form: <username>/feeds/<feedname>
Adafruit_MQTT_Publish soc_feed = Adafruit_MQTT_Publish(&mqtt, AIO_USERNAME "/feeds/battery-soc");
Adafruit_MQTT_Publish cell_voltage_feed = Adafruit_MQTT_Publish(&mqtt, AIO_USERNAME "/feeds/battery-cell-voltage");
Adafruit_MQTT_Publish co2_ppm_feed = Adafruit_MQTT_Publish(&mqtt, AIO_USERNAME "/feeds/co2");
Adafruit_MQTT_Publish air_temp_feed = Adafruit_MQTT_Publish(&mqtt, AIO_USERNAME "/feeds/air-temperature");

/*************************** Sketch Code ************************************/

void MQTT_connect();
void getCCS811DataAndPublish();
void getBME280DataAndPublish();
void getGGDataAndPublish();
bool bme280_init();
bool ccs811_init();
bool gg_init();

void setup() {
  Serial.begin(115200);
  delay(10);

  Serial.println("Starting up...");
  Serial.print("Boot count is: ");
  Serial.println(bootCount);
  bootCount++;

  ccs811_init();

  bme280_init();

  gg_init();


  // Connect to WiFi access point.
  Serial.println(); Serial.println();
  Serial.print("Connecting to ");
  Serial.println(WLAN_SSID);

  WiFi.begin(WLAN_SSID, WLAN_PASS);
  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }
  Serial.println();

  Serial.println("WiFi connected");
  Serial.println("IP address: "); Serial.println(WiFi.localIP());

  client.setCACert(test_root_ca);
}

void loop() {
  // Ensure the connection to the MQTT server is alive (this will make the first
  // connection and automatically reconnect when disconnected).  See the MQTT_connect
  // function definition further below.
  MQTT_connect();

  // Now we can publish stuff!
  getGGDataAndPublish();
  getCCS811DataAndPublish();
  getBME280DataAndPublish();

  Serial.print("Going to sleep for (seconds): ");
  Serial.println(TIME_TO_SLEEP);
  esp_sleep_enable_timer_wakeup(TIME_TO_SLEEP * uS_TO_S_FACTOR);
  esp_deep_sleep_start();
}

// Function to connect and reconnect as necessary to the MQTT server.
// Should be called in the loop function and it will take care if connecting.
void MQTT_connect() {
  int8_t ret;

  // Stop if already connected.
  if (mqtt.connected()) {
    return;
  }

  Serial.print("Connecting to MQTT... ");

  uint8_t retries = 3;
  while ((ret = mqtt.connect()) != 0) { // connect will return 0 for connected
       Serial.println(mqtt.connectErrorString(ret));
       Serial.println("Retrying MQTT connection in 5 seconds...");
       mqtt.disconnect();
       delay(5000);  // wait 5 seconds
       retries--;
       if (retries == 0) {
         // basically die and wait for WDT to reset me
         while (1);
       }
  }
  Serial.println("MQTT Connected!");
}

void getCCS811DataAndPublish() {
    CCS811.writeBaseLine(CCS811_BASELINE);

    Serial.println("Waiting for CCS811 data to be ready");
    while (!CCS811.checkDataReady()) {
      delay(100);
    }

    double co2_ppm = CCS811.getCO2PPM();
    Serial.print("\nSending co2 ppm val ");
    Serial.print(co2_ppm);
    Serial.print("...");
    if (! co2_ppm_feed.publish(co2_ppm)) {
      Serial.println("Failed");
    } else {
      Serial.println("OK!");
    }
}

void getBME280DataAndPublish() {
  float temp = bme.getTemperature();
  Serial.print("\nSending air temp val ");
  Serial.print(temp);
  Serial.print("...");
  if (! air_temp_feed.publish(temp)) {
    Serial.println("Failed");
  } else {
    Serial.println("OK!");
  }
}

// show last sensor operate status
void printLastOperateStatus(BME::eStatus_t eStatus)
{
  switch(eStatus) {
  case BME::eStatusOK:    Serial.println("everything ok"); break;
  case BME::eStatusErr:   Serial.println("unknow error"); break;
  case BME::eStatusErrDeviceNotDetected:    Serial.println("device not detected"); break;
  case BME::eStatusErrParameter:    Serial.println("parameter error"); break;
  default: Serial.println("unknow status"); break;
  }
}

bool bme280_init()
{
  int setupRetries;

  bme.reset();

  Serial.println("Setting up bme280");
  for (setupRetries = 0; setupRetries < NUM_SETUP_RETRIES; setupRetries++) {
    if (bme.begin() != BME::eStatusOK) {
      Serial.println("bme begin faild");
      printLastOperateStatus(bme.lastOperateStatus);
      delay(500);
    } else {
      break;
    }
  }

  if (setupRetries >= NUM_SETUP_RETRIES) {
    Serial.println("Gave up setting up bme280");
    while (1);
  }

  Serial.println("bme init success");

  return true;
}

bool ccs811_init()
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
    while (1);
  }

  Serial.println("ccs811 init success");

  return true;
}

bool gg_init()
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
    while (1);
  }

  gg.setCellCapacity(LC709203F_APA_2000MAH);

  gg.setAlarmVoltage(3.4);
  gg.setCellProfile( LC709203_NOM3p7_Charge4p2 ) ;

  Serial.println("ccs811 init success");

  return true;
}

void getGGDataAndPublish()
{
  Serial.print("\nSending soc val ");
  double battery_soc = double(gg.cellRemainingPercent10()) / 10;
  Serial.print(battery_soc);
  Serial.print("...");
  if (! soc_feed.publish(battery_soc)) {
    Serial.println("Failed");
  } else {
    Serial.println("OK!");
  }

  Serial.print("\nSending cell voltage val ");
  double battery_voltage_mv = double(gg.cellVoltage_mV()) / 1000.0;
  Serial.print(battery_voltage_mv);
  Serial.print("...");
  if (! cell_voltage_feed.publish(battery_voltage_mv)) {
    Serial.println("Failed");
  } else {
    Serial.println("OK!");
  }
}
