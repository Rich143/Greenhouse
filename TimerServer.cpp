#include "TimerServer.h"
#include "Logger.h"

void TimerServer::init()
{
    configTime(_gmtOffsetSeconds, _daylightOffsetSeconds, _ntpServer);
}

status_t TimerServer::getTime(struct tm *tm)
{
    if (tm == nullptr) {
        LOG_ERROR("getTime received null time info struct");
        return STATUS_INVALID_PARAMS;
    }

    if (!getLocalTime(tm)) {
        LOG_ERROR("Failed to get local time");
        return STATUS_FAIL;
    }

    return STATUS_OK;
}

