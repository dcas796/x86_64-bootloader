#ifndef TYPES_H
#define TYPES_H

#define NULL 0

typedef unsigned char       uint8_t;
typedef unsigned short      uint16_t;
typedef unsigned int        uint32_t;
typedef unsigned long long  uint64_t;
typedef char                int8_t;
typedef short               int16_t;
typedef int                 int32_t;
typedef long long           int64_t;
typedef uint32_t            size_t;

#define PACKED __attribute__((packed))

#endif
