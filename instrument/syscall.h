// Minimun syscall wrapper. 
//XXX: How to do syscalls without libc?

#include <stdint.h>
#include <stddef.h>

size_t bfl_read(int fd, char* dst, size_t len);

uint64_t bfl_open(char* filename, uint64_t flag, uint64_t mode);

size_t bfl_write(int fd, char* dmst, size_t len);

int64_t bfl_shmat(int shm_id);

uint64_t bfl_wait4(int id, int* infop, int options, int rusage);

uint64_t bfl_exit(uint64_t errer_code);

uint64_t bfl_close(int fd);

int bfl_fork();
