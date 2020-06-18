#include "syscall.h"
#include "libc_funcs.h"

#define AFL_SHM_ENV "__AFL_SHM_ID"
#define FORKSRV_FD 198  


char *__afl_area_ptr = 0;
uint64_t __afl_prev_loc = 0;
char *__afl_global_area_ptr = 0;
int32_t __afl_temp = 0;
uint64_t __afl_fork_pid = 0;



void _afl_maybe_log(uint64_t instr_addr)
{
    #ifdef DEBUG
    debug_print((void*)instr_addr);
    #endif
    if (__afl_area_ptr == 0)
        return;
    instr_addr = (instr_addr >> 1 | (instr_addr << 63));
    instr_addr %= 0x10000;
    instr_addr ^= __afl_prev_loc;
    __afl_prev_loc ^= instr_addr;
    instr_addr >>= 1;
    __afl_area_ptr[instr_addr+1] += 1;
    return;
}


void fork_server(int argc, char** argv, char** envp){
    // setup shm, init ptrs
    char* afl_env = _getenv(AFL_SHM_ENV, envp);
    if (afl_env == NULL)
        return;
    uint64_t shm_id = _atol(afl_env);
    int64_t result = bfl_shmat(shm_id);
    if (result <= 0)
        return;
    __afl_area_ptr = (char*)result;
    __afl_global_area_ptr = __afl_area_ptr;

    // handshake with afl
    size_t wlen = bfl_write(FORKSRV_FD+1, (char*)&__afl_temp,4);
    if(wlen!=4)
        goto __afl_fork_resume;
    while (1)
    {
        size_t rlen = bfl_read(FORKSRV_FD, (char*)&__afl_temp, 4);
        if(rlen!=4)
            _abort();
        uint64_t pid = bfl_fork();
        if (pid < 0)
            bfl_exit(pid);
        if(pid == 0)
            goto __afl_fork_resume;

        __afl_fork_pid = pid;
        bfl_write(FORKSRV_FD+1, (char*)&__afl_fork_pid, 4);
        int result = bfl_wait4(pid, &__afl_temp, 0, 0);
        if (result <= 0)
        {
            bfl_exit(result);
        }
        bfl_write(FORKSRV_FD+1, (char*)&__afl_temp, 4);
    }

__afl_fork_resume:
    //this part for child process
    bfl_close(FORKSRV_FD);
    bfl_close(FORKSRV_FD+1);
    return;

}



void init(int argc, char** argv, char** envp)
{
    fork_server(argc, argv, envp);
}
