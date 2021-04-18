#ifndef SOILMOISTURE_H_4TALNFYG
#define SOILMOISTURE_H_4TALNFYG

#include "Arduino.h"

#define SOIL_MOISTURE_READING_AIR 3575
#define SOIL_MOISTURE_READING_WATER 100

#define SOIL_MOISTURE_PIN 34

/*!
 *  \class SoilMoisture
 *  \brief Soil moisture monitoring via DFRobot waterproof capacitive soil
 *  moisture sensor
 *
 */
class SoilMoisture
{
public:
    SoilMoisture() :
        _reading_air(SOIL_MOISTURE_READING_AIR),
        _reading_water(SOIL_MOISTURE_READING_WATER),
        _sensor_pin(SOIL_MOISTURE_PIN) {};

    /**
     * @brief Read the soil moisture as a percentage between dry (in air) vs
     * wet (in water)
     *
     * @return Soil moisture percentage
     */
    double soilMoisturePercent();

protected:

    /**
     * @brief Convert the adc reading to percentage
     *
     * This essentially maps the adc val between the wet and dry values
     *
     * @param adcVal The sensor adc reading
     *
     * @return The soil moisture percentage
     */
    double soilMoistureADCValToPercent(double adcVal);

    double mapRange(double in, double inMin, double inMax, double outMin, double outMax);

    //! The adc reading in air
    uint32_t _reading_air;

    //! The adc reading in water
    uint32_t _reading_water;

    //! The pin the sensor is connected to
    uint32_t _sensor_pin;
};

#endif /* end of include guard: SOILMOISTURE_H_4TALNFYG */
