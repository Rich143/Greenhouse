#include "Thermistor.h"
#include "Logger.h"

// Number of ADC samples to average for each reading
#define THERMISTOR_NUM_ADC_SAMPLES 10
// The voltage input to the thermistor voltage divider, in volts
#define THERMISTOR_VOLTAGE_INPUT_VOLTS (3.3f)

/** 
 * @brief Function to more accurately convert adc value to voltage
 *
 * From: https://github.com/G6EJD/ESP32-ADC-Accuracy-Improvement-function
 * A function that improves the default ADC reading accuracy to within 1%
 *
 * The ESP32 ADC has two non-linear regions, one just below ~0.5v and the other
 * just above ~2.5v, however by using a polynomial the output can be made
 * accurate to within 1% by adjusting the ADC output to the correct value.
 *
 * IMPORTANT: This function assumes that the voltage reference is 3.3V
 * 
 * @param adcValue The raw adc value from 0-4095
 * 
 * @return voltage measured in volts
 */
double Thermistor::adcValueToVoltage(uint32_t adcValue){
  if (adcValue < 1 || adcValue > 4095) {
      return 0;
  }

  return -0.000000000000016 * pow(adcValue,4) + 0.000000000118171 *
      pow(adcValue,3)- 0.000000301211691 * pow(adcValue,2)+ 0.001109019271794 *
      adcValue + 0.034143524634089;
}

double Thermistor::readVoltage() {
    double average_voltage = 0;

    for (int i = 0; i < THERMISTOR_NUM_ADC_SAMPLES; ++i) {
        uint32_t value = analogRead(_thermistor_adc_pin);
        double voltage = adcValueToVoltage(value);

        average_voltage += voltage;
    }

    average_voltage /= THERMISTOR_NUM_ADC_SAMPLES;

    return average_voltage;
}
double Thermistor::adcVoltageToResistance(double voltage) {
    double resistance = voltage * _nominal_resistance;
    resistance /= (THERMISTOR_VOLTAGE_INPUT_VOLTS - voltage);

    return resistance;
}

double Thermistor::readTemperature() {
    double voltage = readVoltage();
    double resistance = adcVoltageToResistance(voltage);
    double temperature = resistanceToTemperature(resistance);

    return temperature;
}

//double Thermistor::readADCValue() {
    //double average = 0;

    //for (int i = 0; i < THERMISTOR_NUM_ADC_SAMPLES; i++) {
        //uint32_t value = analogRead(_thermistor_adc_pin);
        //LOG_DEBUG("Thermistor adc val " + String(value));

        //average += value;
        //// Small delay to help average out noise
        //delay(10);
    //}

    //average /= THERMISTOR_NUM_ADC_SAMPLES;

    //LOG_DEBUG("Thermistor adcValue " + String(average));
    //return average;
//}

//double Thermistor::adcValueToResistance(double adcValue) {
    //double resistance = (4095 / adcValue) - 1;
    //resistance = _series_resistance / resistance;

    //LOG_DEBUG("Thermistor resistance " + String(resistance));
    //return resistance;
//}

double Thermistor::resistanceToTemperature(double resistance) {
    // Calculate thermistor temperature using Steinhart-Hart equation
    double temperature;
    temperature = resistance / _nominal_resistance;       // (R/Ro)
    temperature = log(temperature);                       // ln(R/Ro)
    temperature /= _b_value;                              // 1/B * ln(R/Ro)
    temperature += 1.0 / (_nominal_temperature + 273.15); // + (1/To)
    temperature = 1.0 / temperature;                      // Invert
    temperature -= 273.15;                                // convert absolute temp to C

    return temperature;
}
