#include "SoilMoisture.h"
#include "Logger.h"
#include "Utilities.h"

//! Number of ADC readings to average per soil moisture reading
#define SOIL_MOISTURE_NUMBER_ADC_READINGS 100

double SoilMoisture::soilMoistureADCValToPercent(double adcVal) {
    // Moisture sensor increases in value as moisture decreases, giving max
    // reading in air
    double moisturePercent = (100 - mapRange(adcVal, _reading_water, _reading_air, 0, 100));
    
    return moisturePercent;
}

double SoilMoisture::soilMoisturePercent() {
    double average = 0;

    for (int i = 0; i < SOIL_MOISTURE_NUMBER_ADC_READINGS; ++i) {
        average += analogRead(_sensor_pin);
        delay(10);
    }

    average /= SOIL_MOISTURE_NUMBER_ADC_READINGS;

    return soilMoistureADCValToPercent(average);
}
