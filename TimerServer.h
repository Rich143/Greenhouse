#ifndef TIMERSERVER_H_KQFGXLRA
#define TIMERSERVER_H_KQFGXLRA

#include "Arduino.h"
#include "Status.h"

/*! \class TimerServer
 *  \brief Class to manage system time based on NTP server
 *
 *  Detailed description
 */
class TimerServer
{
public:
    void init();

    struct tm getLocalTime();

protected:
    const char *_ntpServer = "pool.ntp.org"; // NTP server url
    const long _gmtOffsetSeconds = -18000; // UTC-5
    const int  _daylightOffsetSeconds = 3600;
};

#endif /* end of include guard: TIMERSERVER_H_KQFGXLRA */
