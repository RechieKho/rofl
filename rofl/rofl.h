#ifndef _ROFL_H_
#define _ROFL_H_

#include <stdbool.h>

// Structs.
// Slice of string.
struct rofl_slice {
    const char* begin;
    unsigned int length;
};

// Enums.
enum rofl_error {
    ROFL_OK = 0,
    ROFL_ERROR_SYNTAX,
    ROFL_ERROR_INTERNAL,
    ROFL_ERROR_UNEXPECTED_MEMBER,
    ROFL_ERROR_UNEXPECTED_VALUE,
};

// Types.
typedef enum rofl_error (*rofl_callback_t)(
        const struct rofl_slice* p_object, 
        unsigned int p_nest_count,
        const struct rofl_slice p_member_name, 
        const struct rofl_slice p_hint,
        const double* p_number_values, 
        unsigned int p_number_values_count,
        const struct rofl_slice* p_string_values,
        unsigned int p_string_values_count
);

// Routine.
// Parse rofl.
enum rofl_error parse_rofl(
    const char* p_string, 
    rofl_callback_t p_callback,
    bool p_quiet
);

#endif //_ROFL_H_
