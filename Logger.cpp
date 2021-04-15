#include <cstdio>

#include "Logger.h"
#include "config.h"

Logger gLogger(DEFAULT_LOG_LEVEL_MQTT, DEFAULT_LOG_LEVEL_UART);

Logger::Logger(LogLevel mqttLevel, LogLevel uartLevel) {
    _levels[MQTT] = mqttLevel;
    _levels[UART] = uartLevel;

    _loggingFeed = nullptr;

    _mqttEnabled = false;
}

status_t Logger::enableMqttLogging(Adafruit_MQTT_Publish *loggingFeed) {
    if (loggingFeed == nullptr) {
        Serial.println("Attempt to enable MQTT logging with null feed");
        return STATUS_INVALID_PARAMS;
    }

    _loggingFeed = loggingFeed;
    _mqttEnabled = true;
}

status_t Logger::LogFormatted(LogLevel level, const char *fmt, ...) {
    va_list pargs;

    va_start(pargs, fmt);
    vsnprintf(_logFormatBuffer, LOG_FORMAT_BUFFER_LEN, fmt, pargs);
    va_end(pargs);

    return Log(level, _logFormatBuffer);
}

status_t Logger::Log(LogLevel level, String msg) {
    for (int output = 0; output < NUM_LOGGING_OUTPUTS; ++output) {
        status_t rc;

        if (canLog(level, static_cast<LoggingOutputs>(output))) {
            switch (output) {
                case MQTT:
                    rc = logMQTT(msg);
                    if (rc != STATUS_OK) {
                        return rc;
                    }
                    break;
                case UART:
                    rc = logUART(msg);
                    if (rc != STATUS_OK) {
                        return rc;
                    }
                    break;
                default:
                    Serial.println("Unhandled log output");
                    return STATUS_FAIL;
                    break;
            }
        }
    }

    return STATUS_OK;
}

status_t Logger::logUART(String msg) {
    Serial.println(msg);

    return STATUS_OK;
}

status_t Logger::logMQTT(String msg) {
    if (_loggingFeed == nullptr) {
        Serial.println("Logging MQTT feed null, initialize logging with a MQTT feed");
        return STATUS_FAIL;
    }

    if (!_loggingFeed->publish(msg.c_str())) {
        Serial.println("Failed to log over MQTT");
        return STATUS_FAIL;
    }

    return STATUS_OK;
}

bool Logger::canLog(LogLevel level, LoggingOutputs output) {
    if (output == MQTT && !_mqttEnabled) {
        return false;
    }

    if (level <= _levels[output]) {
        return true;
    } else {
        return false;
    }
}
