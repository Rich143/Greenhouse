#include "Status.h"

String status_to_string(status_t status) {
  switch(status) {
      case STATUS_OK:    return "Everything ok"; break;
      case STATUS_INVALID_PARAMS: return "Invalid Params"; break;
      case STATUS_TIMEOUT: return "Timeout"; break;
      case STATUS_FAIL:  return "Failure"; break;
      default: return "Unknown status"; break;
  }
}
