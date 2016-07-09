#ifndef FM_GPIO_HPP
#define FM_GPIO_HPP

#ifdef PI_NEW_MODEL
#define BCM2708_PERI_BASE        0x3F000000
#else
#define BCM2708_PERI_BASE        0x20000000
#endif
#define GPIO_BASE                (BCM2708_PERI_BASE + 0x200000) /* GPIO controller */

#define PAGE_SIZE (4*1024)
#define BLOCK_SIZE (4*1024)

extern int  mem_fd;
extern void *gpio_map;

// I/O access
extern volatile unsigned *gpio;

#ifndef NOGPIO
// GPIO setup macros. Always use INP_GPIO(x) before using OUT_GPIO(x) or SET_GPIO_ALT(x,y)
#define INP_GPIO(g) *(gpio+((g)/10)) &= ~(7<<(((g)%10)*3))
#define OUT_GPIO(g) *(gpio+((g)/10)) |=  (1<<(((g)%10)*3))
#define SET_GPIO_ALT(g,a) *(gpio+(((g)/10))) |= (((a)<=3?(a)+4:(a)==4?3:2)<<(((g)%10)*3))
#define GET_GPIO(g) (*(gpio+13)&(1<<g)) // 0 if LOW, (1<<g) if HIGH
#else
#define INP_GPIO(g)
#define OUT_GPIO(g)
#define SET_GPIO_ALT(g)
#define GET_GPIO(g) 0
#endif

#define GPIO_SET *(gpio+7)  // sets   bits which are 1 ignores bits which are 0
#define GPIO_CLR *(gpio+10) // clears bits which are 1 ignores bits which are 0


void setup_io();

#endif
