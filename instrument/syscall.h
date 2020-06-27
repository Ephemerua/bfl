// Minimun syscall wrapper. 
//XXX: How to do syscalls without libc?

#include <stdint.h>
#include <stddef.h>

// typedefs
typedef uint8_t u8;
typedef uint16_t u16;
typedef uint32_t u32;
typedef uint64_t u64;

typedef int8_t s8;
typedef int16_t s16;
typedef int32_t s32;
typedef int64_t s64;


size_t bfl_read(s32 fd, u8* dst, size_t len);

size_t bfl_write(int fd, u8* dst, size_t len);

u64 bfl_open(u8* filename, u64 flag, u64 mode);

void* bfl_shmat(int shm_id);

u64 bfl_wait4(int id, int* infop, int options, int rusage);

u64 bfl_exit(s32 errer_code);

s32 bfl_close(s32 fd);

s32 bfl_kill(s32 pid, s32 sig);

s32 bfl_fork();

s32 bfl_getpid();
