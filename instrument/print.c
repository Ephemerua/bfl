#include "syscall.h"


void print_address(int fd, void* addr)
{
    unsigned char buf[0x14];
    uint64_t temp = (uint64_t)addr;
    buf[0] = '0';
    buf[1] = 'x';
    buf[0x12] = '\n';
    for(int i = 2; i < 18; i++){
        buf[i] = '0' + ((temp & 0xf000000000000000)>>(4*15));
        if (buf[i]>'9')
            buf[i] = buf[i] - '9' + 'a' - 1;
        temp <<= 4;
    }
    bfl_write(fd, buf, 0x13);
}

#define PRINT(x) \
do { bfl_write(1, x, sizeof(x))} while(0)
