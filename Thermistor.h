#ifndef THERMISTOR_H_UJAY4WWS
#define THERMISTOR_H_UJAY4WWS

#include "Arduino.h"

/*
 * Default Parameters for thermistor connection.
 * This is the setup for the soil thermistor using an Adafruit Thermistor (10K
 * Precision Epoxy Thermistor - 3950 NTC)
 *
 * This setup uses a voltage divider with a 10K fixed resistor.
 * The fixed resistor is connected to Vcc and the thermistor, and the other
 * thermistor end is connected to ground
 */
#define THERMISTOR_DEFAULT_B_VALUE 3950
#define THERMISTOR_DEFAULT_NOMINAL_RESISTANCE_OHMS 10000
#define THERMISTOR_DEFAULT_SERIES_RESISTOR_OHMS 10000
#define THERMISTOR_DEFAULT_NOMINAL_TEMPERATURE_CELSIUS 25
#define THERMISTOR_ADC_PIN 32

class Thermistor {
    public:

    Thermistor() : _thermistor_adc_pin(THERMISTOR_ADC_PIN),
                   _b_value(THERMISTOR_DEFAULT_B_VALUE),
                   _nominal_resistance(THERMISTOR_DEFAULT_NOMINAL_RESISTANCE_OHMS),
                   _series_resistance(THERMISTOR_DEFAULT_SERIES_RESISTOR_OHMS),
                   _nominal_temperature(THERMISTOR_DEFAULT_NOMINAL_TEMPERATURE_CELSIUS)
    {};

    /**
     * @brief Read the temperature from the thermistor in degrees celsius
     *
     * @return Temperature in degrees celsius
     */
    double readTemperature();

    private:

    /**
     * @brief Read the raw thermistor ADC value
     *
     * @return The raw ADC value (from 0 - 4095)
     */
    //double readADCValue();

    /**
     * @brief Convert a raw ADC Value to the thermistor resistance
     *
     * @param adcValue The raw adc value (from 0 - 4095)
     *
     * @return The resistance in ohms
     */
    //double adcValueToResistance(double adcValue);

    /** 
     * @brief Convert an adc value to the measured voltage
     * 
     * @param adcValue The raw adc value from 0 - 4095
     * 
     * @return The voltage in volts
     */
    double adcValueToVoltage(uint32_t adcValue);

    /** 
     * @brief Convert the measured adc voltage to thermistor resistance
     * 
     * @param voltage The voltage in volts
     * 
     * @return The resistance in ohms
     */
    double adcVoltageToResistance(double voltage);
    
    /** 
     * @brief Read the adc voltage of the thermistor pin
     * 
     * @return The voltage in volts
     */
    double readVoltage();

    /**
     * @brief Convert the thermistor resistance to temperature in degrees
     * celsius
     *
     * @param resistance The resistance of the thermistor in ohms
     *
     * @return The thermistor temperature in degrees celsius
     */
    double resistanceToTemperature(double resistance);

    uint32_t _thermistor_adc_pin;
    uint32_t _b_value;
    uint32_t _nominal_resistance;
    uint32_t _series_resistance;
    uint32_t _nominal_temperature;
};

#endif /* end of include guard: THERMISTOR_H_UJAY4WWS */
