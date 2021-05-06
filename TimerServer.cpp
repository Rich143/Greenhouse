#include "TimerServer.h"

void TimerServer::init()
{
    configTime(_gmtOffsetSeconds, _daylightOffsetSeconds, _ntpServer);
}

struct tm TimerServer::getLocalTime()
{
    return getLocalTime();
}

