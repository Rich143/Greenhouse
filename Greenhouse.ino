/***************************************************
  Greenhouse monitor

 ****************************************************/

/**
 * Library Includes
 */
#include <WiFiClientSecure.h>
#include "Adafruit_MQTT.h"
#include "Adafruit_MQTT_Client.h"

/**
 * Local Includes
 */
#include "config.h"
#include "Secrets.h"
#include "Sensors.h"

/************************* Deep Sleep *********************************/

#define uS_TO_S_FACTOR 1000000  /* Conversion factor for micro seconds to seconds */
#define TIME_TO_SLEEP  60        /* Time ESP32 will go to sleep (in seconds) */

/************************* Other Constants *********************************/

#define NUM_SETUP_RETRIES 5

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

/****************************** Feeds ***************************************/

// Setup a feed called 'photocell' for publishing.
// Notice MQTT paths for AIO follow the form: <username>/feeds/<feedname>
Adafruit_MQTT_Publish soc_feed = Adafruit_MQTT_Publish(&mqtt, AIO_USERNAME "/feeds/battery-soc");
Adafruit_MQTT_Publish cell_voltage_feed = Adafruit_MQTT_Publish(&mqtt, AIO_USERNAME "/feeds/battery-cell-voltage");
Adafruit_MQTT_Publish co2_ppm_feed = Adafruit_MQTT_Publish(&mqtt, AIO_USERNAME "/feeds/co2");
Adafruit_MQTT_Publish air_temp_feed = Adafruit_MQTT_Publish(&mqtt, AIO_USERNAME "/feeds/air-temperature");

/*************************** Sketch Code ************************************/

void MQTT_connect();

// Error handler sleeps to restart us
void errorHandler() {
  esp_sleep_enable_timer_wakeup(TIME_TO_SLEEP * uS_TO_S_FACTOR);
  esp_deep_sleep_start();
}

void setup() {
  Serial.begin(115200);
  delay(10);

  Serial.println("Starting up...");
  Serial.print("Boot count is: ");
  Serial.println(bootCount);
  bootCount++;

  // Connect to WiFi access point.
  Serial.println(); Serial.println();
  Serial.print("Connecting to ");
  Serial.println(WLAN_SSID);

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

  Serial.println("WiFi connected");
  Serial.println("IP address: "); Serial.println(WiFi.localIP());

  client.setCACert(test_root_ca);

  Serial.println("Initializing sensors");
  sensor_status_t rc = gSensors.init();
  if (rc != SENSOR_OK) {
    Serial.print("Error initing sensors: ");
    Serial.println(Sensors::status_to_string(rc));
    errorHandler();
  }

  Serial.println("Setting sensor mqtt feeds");
  gSensors.set_soc_feed(&soc_feed);
  gSensors.set_cell_voltage_feed(&cell_voltage_feed);
  gSensors.set_co2_feed(&co2_ppm_feed);
  gSensors.set_air_temp_feed(&air_temp_feed);
}

void loop() {
  // Ensure the connection to the MQTT server is alive (this will make the first
  // connection and automatically reconnect when disconnected).  See the MQTT_connect
  // function definition further below.
  MQTT_connect();

  Serial.println("Reading from sensors");
  sensor_status_t rc = gSensors.update_all_values();
  if (rc != SENSOR_OK) {
    Serial.print("Error reading sensors: ");
    Serial.println(Sensors::status_to_string(rc));
    errorHandler();
  }

  Serial.println("Publishing sensor data");
  rc = gSensors.publish_all_feeds();
  if (rc != SENSOR_OK) {
    Serial.print("Error publishing sensor data: ");
    Serial.println(Sensors::status_to_string(rc));
    errorHandler();
  }

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
         errorHandler();
       }
  }
  Serial.println("MQTT Connected!");
}
