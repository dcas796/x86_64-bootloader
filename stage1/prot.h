#ifndef PROT_H
#define PROT_H

#include <sysinfo.h>

typedef enum {
    CONTROL_SUCCESS,
    CONTROL_NO_SPACE_FOR_STACK,
} control_result_t;

control_result_t transfer_control(void *entry_point, const sysinfo_t *info);
const char *control_result_to_str(control_result_t result);

#endif