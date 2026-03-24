#ifndef CONST_H
#define CONST_H

#define STAGE0_PHY_ADDR     0x7c00
#define STAGE0_END          (STAGE0_PHY_ADDR + 512)

#define STAGE1_OFFSET       0x8000
#define STAGE1_SEGMENT      0x0000
#define STAGE1_PHY_ADDR     ((STAGE1_SEGMENT << 4) + STAGE1_OFFSET)
#define STAGE1_STACK_TOP    STAGE1_OFFSET
#define STAGE1_STACK_BASE   0x7000

#define REAL_MODE_LIMIT     ((0xffff << 4) + 0xffff)

#define EXTENDED_STACK_BASE  0x80000
#define EXTENDED_STACK_LIMIT 0x40000
#define STATIC_STACK_BASE    EXTENDED_STACK_LIMIT
#define STATIC_STACK_LIMIT   0x10000


#define PROTECTED_MODE_STACK_TOP 0xc00000 /* TODO: put this at the top of memory */

#endif
