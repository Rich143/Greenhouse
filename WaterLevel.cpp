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
                                          _distanceFullCm.getValue(),
                                          _distanceEmptyCm.getValue(),
                                          0, 100);

    *waterLevelPercentOut = water_level_percent;

    return STATUS_OK;
}

status_t WaterLevel::init()
{
    status_t rc;

    rc = _distanceEmptyCm.initAndLoad(
        "waterEmpty", DEFAULT_WATERLEVEL_EMPTY_LEVEL_DISTANCE_CM);
    if (rc != STATUS_OK) {
        LOG_ERROR("Failed to load water level empty config value");
        return rc;
    }

    rc = _distanceFullCm.initAndLoad(
        "waterFull", DEFAULT_WATERLEVEL_FULL_LEVEL_DISTANCE_CM);
    if (rc != STATUS_OK) {
        LOG_ERROR("Failed to load water level full config value");
        return rc;
    }

    return STATUS_OK;
}

status_t WaterLevel::updateWaterLevelCalibration(uint32_t distanceFullCm,
                                     uint32_t distanceEmptyCm)
{
    status_t rc;

    rc = _distanceEmptyCm.updateValue(distanceEmptyCm);
    if (rc != STATUS_OK) {
        LOG_ERROR("Failed to update distance empty cm");
        return rc;
    }

    rc = _distanceFullCm.updateValue(distanceFullCm);
    if (rc != STATUS_OK) {
        LOG_ERROR("Failed to update distance full cm");
        return rc;
    }

    return STATUS_OK;
}
