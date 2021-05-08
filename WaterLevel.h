#ifndef WATERLEVEL_H_YNGUBFMN
#define WATERLEVEL_H_YNGUBFMN

#include "Arduino.h"
#include <NewPing.h>
#include "Status.h"
#include "ConfigValue.h"

#define WATERLEVEL_DEFAULT_TRIG_PIN 27
#define WATERLEVEL_DEFAULT_ECHO_PIN 25

//! Value measured by sonar sensor when the water reservoir is empty
#define DEFAULT_WATERLEVEL_EMPTY_LEVEL_DISTANCE_CM 60
//! Value measured by sonar sensor when the water reservoir is full
#define DEFAULT_WATERLEVEL_FULL_LEVEL_DISTANCE_CM 5

/**
 * The max ping distance is limited by the greenhouse height, so set to 2m as
 * this is more than we'll ever get
 */
#define WATERLEVEL_MAX_VALID_PING_DISTANCE_CM 200

/*!
 * \class WaterLevel
 * \brief Class to measure water level using ultrasonic sensor
 */
class WaterLevel : public NewPing
{
public:
    WaterLevel():
        NewPing(WATERLEVEL_DEFAULT_TRIG_PIN, WATERLEVEL_DEFAULT_ECHO_PIN,
                WATERLEVEL_MAX_VALID_PING_DISTANCE_CM) {};

    status_t getWaterLevelPercent(double *waterLevelPercentOut);

    uint32_t getWaterDistanceCm();

    status_t init();

    status_t updateWaterLevelCalibration(uint32_t distanceFullCm,
                                         uint32_t distanceEmptyCm);
protected:
    ConfigValue _distanceEmptyCm;
    ConfigValue _distanceFullCm;
};

#endif /* end of include guard: WATERLEVEL_H_YNGUBFMN */
