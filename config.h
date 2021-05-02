#ifndef __CONFIG_H
#define __CONFIG_H

#include "Secrets.h"

/************************* WiFi Access Point *********************************/

#define WLAN_SSID       "Matthews Home Network"
#define WLAN_PASS       "OurNetwork@Home"

/************************* Adafruit.io Setup *********************************/

#define AIO_SERVER      "io.adafruit.com"
#define AIO_SERVERPORT  8883                   // use 8883 for SSL, or 1883 for non secure
#define AIO_USERNAME    "Rich_M"

/************************* Logging *********************************/
#define DEFAULT_LOG_LEVEL_UART Logger::INFO

#define DEFAULT_LOG_LEVEL_MQTT Logger::WARN

#endif /* end of include guard: __CONFIG_H */
