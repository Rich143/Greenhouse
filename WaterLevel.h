#ifndef WATERLEVEL_H_YNGUBFMN
#define WATERLEVEL_H_YNGUBFMN

#include "Arduino.h"
#include <NewPing.h>
#include "Status.h"

#define WATERLEVEL_DEFAULT_TRIG_PIN 27
#define WATERLEVEL_DEFAULT_ECHO_PIN 25

//! Value measured by sonar sensor when the water reservoir is empty
#define WATERLEVEL_EMPTY_LEVEL_DISTANCE_CM 60
//! Value measured by sonar sensor when the water reservoir is full
#define WATERLEVEL_FULL_LEVEL_DISTANCE_CM 5
/**
 * Note: We initialize the max ping distance to the empty distance plus a small
 * margin, as no valid pings should be larger than that value
 * The margin is needed as we keep pinging until we get a valid distance, so we
 * add a margin to ensure we always can get a valid distance
 */
#define WATERLEVEL_MAX_VALID_PING_DISTANCE_CM (WATERLEVEL_EMPTY_LEVEL_DISTANCE_CM + 5)

/*!
 * \class WaterLevel
 * \brief Class to measure water level using ultrasonic sensor
 */
class WaterLevel : public NewPing
{
public:
    WaterLevel():
        _full_level(WATERLEVEL_FULL_LEVEL_DISTANCE_CM),
        _empty_level(WATERLEVEL_EMPTY_LEVEL_DISTANCE_CM),
        NewPing(WATERLEVEL_DEFAULT_TRIG_PIN, WATERLEVEL_DEFAULT_ECHO_PIN,
                WATERLEVEL_MAX_VALID_PING_DISTANCE_CM) {};

    status_t getWaterLevelPercent(double *waterLevelPercentOut);

protected:
    uint32_t getWaterDistanceCm();

    const uint32_t _full_level;
    const uint32_t _empty_level;
};

#endif /* end of include guard: WATERLEVEL_H_YNGUBFMN */
