#include "WaterLevel.h"
#include "Utilities.h"
#include "Logger.h"

#define WATERLEVEL_NUM_PINGS_PER_MEASUREMENT 10

uint32_t WaterLevel::getWaterDistanceCm()
{
    uint32_t us = ping_median(WATERLEVEL_NUM_PINGS_PER_MEASUREMENT);

    uint32_t distance_cm = convert_cm(us);

    return distance_cm;
}

status_t WaterLevel::getWaterLevelPercent(double *waterLevelPercentOut)
{
    if (waterLevelPercentOut == nullptr) {
        LOG_ERROR("Null output parameter passed to getWaterLevelPercent");
        return STATUS_INVALID_PARAMS;
    }

    uint32_t distance_cm = getWaterDistanceCm();

    if (distance_cm == NO_ECHO) {
        return STATUS_TIMEOUT;
    }

    // Map the distance to a percentage, then invert since further distances
    // are lower water levels
    double water_level_percent = 100 - mapRange(distance_cm,
                                          WATERLEVEL_FULL_LEVEL_DISTANCE_CM,
                                          WATERLEVEL_EMPTY_LEVEL_DISTANCE_CM,
                                          0, 100);

    *waterLevelPercentOut = water_level_percent;

    return STATUS_OK;
}
