#ifndef UTILITIES_H_EERAPQH3
#define UTILITIES_H_EERAPQH3

/** 
 * @brief An version of the arduino map function, using doubles. It maps a
 * value from one range to another
 * 
 * @param in The value to map
 * @param inMin The minimum value of the input range
 * @param inMax The maximum value of the input range
 * @param outMin The maximum value of the output range
 * @param outMax The maximum value of the output range
 * 
 * @return The input value mapped to the output range
 */
double mapRange(double in, double inMin, double inMax, double outMin, double outMax);

#endif /* end of include guard: UTILITIES_H_EERAPQH3 */
