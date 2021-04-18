#include "Utilities.h"

double mapRange(double in, double inMin, double inMax, double
                              outMin, double outMax)
{
    if (in < inMin) {
        return outMin;
    }

    if (in > inMax) {
        return outMax;;
    }

    return outMin + ((in - inMin) * (outMax - outMin)) / (inMax - inMin);
}

