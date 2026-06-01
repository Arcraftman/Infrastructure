#include "stk/def.h"
#include "stk/utils/preset.h"
#include "stk/utils/status.h"

const char* stk_status_str(STK_STATUS status) {
    switch (status) {
        case STK_OK:         return "OK";
        case STK_ERROR:      return "generic error";
        case STK_ENOMEM:     return "out of memory";
        case STK_EINVAL:     return "invalid argument";
        case STK_ENOTFOUND:  return "element not found";
        case STK_EEXISTS:    return "element already exists";
        case STK_ERANGE:     return "index out of range";
        case STK_EBUSY:      return "resource busy";
        case STK_EPERM:      return "operation not permitted";
        case STK_EMPTY:      return "container empty";
        default:             return "unknown status";
    }
}