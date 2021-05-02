#ifndef LOGGER_H_1XENXW60
#define LOGGER_H_1XENXW60

#include <Arduino.h>
#include "Adafruit_MQTT.h"
#include "Adafruit_MQTT_Client.h"
#include <cstdarg>

#include "Status.h"

#define LOG_FORMAT_BUFFER_LEN 200

#define LOG_ERROR(msg) gLogger.Log(Logger::ERROR, msg)
#define LOG_WARN(msg) gLogger.Log(Logger::WARN, msg)
#define LOG_INFO(msg) gLogger.Log(Logger::INFO, msg)
#define LOG_DEBUG(msg) gLogger.Log(Logger::DEBUG, msg)

//#define LOG_ERROR(msg, ...) gLogger.LogFormatted(Logger::ERROR, msg, ##__VA_ARGS__)
//#define LOG_WARN(msg, ...) gLogger.LogFormatted(Logger::WARN, msg, ##__VA_ARGS__)
//#define LOG_INFO(msg, ...) gLogger.LogFormatted(Logger::INFO, msg, ##__VA_ARGS__)
//#define LOG_DEBUG(msg, ...) gLogger.LogFormatted(Logger::DEBUG, msg, ##__VA_ARGS__)

class Logger {
    public:
    enum LogLevel {
        DONT_LOG,
        ERROR,
        WARN,
        INFO,
        DEBUG
    };

    enum LoggingOutputs {
        MQTT,
        UART,
        NUM_LOGGING_OUTPUTS,
    };

    Logger(LogLevel mqttLevel, LogLevel uartLevel);

    //status_t LogFormatted(LogLevel level, const char *fmt, ...);

    status_t Log(LogLevel level, String msg);

    status_t SetLogLevel(LoggingOutputs output, LogLevel level);

    status_t enableMqttLogging(Adafruit_MQTT_Publish *loggingFeed);

    protected:

    bool canLog(LogLevel level, LoggingOutputs output);

    status_t logUART(String msg);
    status_t logMQTT(String msg);

    LogLevel _levels[NUM_LOGGING_OUTPUTS];
    Adafruit_MQTT_Publish *_loggingFeed;

    bool _mqttEnabled;

    char _logFormatBuffer[];
};

extern Logger gLogger;
#endif /* end of include guard: LOGGER_H_1XENXW60 */
