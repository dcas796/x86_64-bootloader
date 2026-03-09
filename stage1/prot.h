#ifndef PROT_H
#define PROT_H

#include <sysinfo.h>

void transfer_control(void *entry_point, const sysinfo_t *info);

#endif