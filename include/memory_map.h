#ifndef CONST_H
#define CONST_H

#define STAGE0_PHY_ADDR     0x7c00
#define STAGE0_END          STAGE0_PHY_ADDR + 512

#define STAGE1_OFFSET       0x8000
#define STAGE1_SEGMENT      0x0000
#define STAGE1_PHY_ADDR     (STAGE1_SEGMENT << 4) + STAGE1_OFFSET
#define STAGE1_STACK_BASE   STAGE1_OFFSET   /* i need to do something better */
#define STAGE1_STACK_TOP    0x7000      /* I'm pulling these values out of my ass */

#define FREE_MEM_ADDR       0x10000     /* literally out of my butt */

#endif
