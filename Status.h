#ifndef STATUS_H_PO4WRDEH
#define STATUS_H_PO4WRDEH

#include "Arduino.h"

enum status_t {
    STATUS_OK,
    STATUS_INVALID_PARAMS,
    STATUS_FAIL,
    STATUS_TIMEOUT,
};

String status_to_string(status_t status);

#endif /* end of include guard: STATUS_H_PO4WRDEH */
