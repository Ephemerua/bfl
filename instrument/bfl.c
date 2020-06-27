#include "syscall.h"
#include "libc_funcs.h"
#include <signal.h>
#include <sys/wait.h>

#define SHM_ENV_VAR         "__AFL_SHM_ID"
#define AS_LOOP_ENV_VAR     "__AFL_AS_LOOPCHECK"
#define PERSIST_ENV_VAR     "__AFL_PERSISTENT"
#define DEFER_ENV_VAR       "__AFL_DEFER_FORKSRV"
#define FORKSRV_FD          198  
#define MAP_SIZE            (1<<16)

u8 *__afl_area_ptr = 0;
_Thread_local u64 __afl_prev_loc = 0;
u8 is_persistent = 0;
char **environment;



void _afl_maybe_log(u64 instr_addr)
{
    #ifdef DEBUG
        PRINT("_afl_maybe_log: instr_addr ");
        print_address((void*)instr_addr);
    #endif

    if (__afl_area_ptr == 0)
        return;

    instr_addr = (instr_addr >> 1 | (instr_addr << 63));
    instr_addr %= MAP_SIZE;
    instr_addr ^= __afl_prev_loc;
    #ifdef DEBUG
        PRINT("_afl_maybe_log: bitmap_offset ");
        print_address((void*)instr_addr);
    #endif
    __afl_prev_loc ^= instr_addr;
    __afl_area_ptr[instr_addr] += 1;
    return;
}


static void __afl_map_shm(void) {

  u8 *id_str = (u8*)_getenv(SHM_ENV_VAR, environment);

  /* If we're running under AFL, attach to the appropriate region, replacing the
     early-stage __afl_area_initial region that is needed to allow some really
     hacky .init code to work correctly in projects such as OpenSSL. */

  if (id_str) {

    u64 shm_id = _atol(id_str);

    __afl_area_ptr = bfl_shmat(shm_id);

    /* Whooooops. */

    if (__afl_area_ptr == (void *)-1) bfl_exit(1);

    /* Write something into the bitmap so that even with low AFL_INST_RATIO,
       our parent doesn't give up on us. */

    __afl_area_ptr[0] = 1;

  }

}


// copy from afl/llvm_mode
static void __afl_start_forkserver(void) {

  static u8 tmp[4];
  s32 child_pid;

  u8  child_stopped = 0;

  /* Phone home and tell the parent that we're OK. If parent isn't there,
     assume we're not running in forkserver mode and just execute program. */

  if (bfl_write(FORKSRV_FD + 1, tmp, 4) != 4) return;

  while (1) {

    u32 was_killed;
    int status;

    /* Wait for parent by reading from the pipe. Abort if read fails. */

    if (bfl_read(FORKSRV_FD, (u8*)&was_killed, 4) != 4) bfl_exit(1);

    /* If we stopped the child in persistent mode, but there was a race
       condition and afl-fuzz already issued SIGKILL, write off the old
       process. */

    if (child_stopped && was_killed) {
      child_stopped = 0;
      if (bfl_wait4(child_pid, &status, 0, 0) < 0) bfl_exit(1);
    }

    if (!child_stopped) {

      /* Once woken up, create a clone of our process. */

      child_pid = bfl_fork();
      if (child_pid < 0) bfl_exit(1);

      /* In child process: close fds, resume execution. */

      if (!child_pid) {

        bfl_close(FORKSRV_FD);
        bfl_close(FORKSRV_FD + 1);
        return;
  
      }

    } else {

      /* Special handling for persistent mode: if the child is alive but
         currently stopped, simply restart it with SIGCONT. */

      bfl_kill(child_pid, SIGCONT);
      child_stopped = 0;

    }

    /* In parent process: write PID to pipe, then wait for child. */

    if (bfl_write(FORKSRV_FD + 1, (u8*)&child_pid, 4) != 4) bfl_exit(1);

    if (bfl_wait4(child_pid, &status, is_persistent ? WUNTRACED : 0, 0) < 0)
      bfl_exit(1);

    /* In persistent mode, the child stops itself with SIGSTOP to indicate
       a successful run. In this case, we want to wake it up without forking
       again. */

    if (WIFSTOPPED(status)) child_stopped = 1;

    /* Relay wait status to pipe, then loop back. */

    if (bfl_write(FORKSRV_FD + 1, (u8*)&status, 4) != 4) bfl_exit(1);

  }

}

/* This one can be called from user code when deferred forkserver mode
    is enabled. */

void __afl_manual_init(void) {

  static u8 init_done;

  if (!init_done) {

    __afl_map_shm();
    __afl_start_forkserver();
    init_done = 1;

  }

}


static void __afl_auto_init(void) 
{

  is_persistent = !!_getenv(PERSIST_ENV_VAR, environment);

  if (_getenv(DEFER_ENV_VAR, environment)) return;

  __afl_manual_init();

}

/* A simplified persistent mode handler, used as explained in README.llvm. */

int __afl_persistent_loop(unsigned int max_cnt) {

  static u8  first_pass = 1;
  static u32 cycle_cnt;

  if (first_pass) {

    /* Make sure that every iteration of __AFL_LOOP() starts with a clean slate.
       On subsequent calls, the parent will take care of that, but on the first
       iteration, it's our job to erase any trace of whatever happened
       before the loop. */

    if (is_persistent) {

      _memset(__afl_area_ptr, 0, MAP_SIZE);
      __afl_area_ptr[0] = 1;
      __afl_prev_loc = 0;
    }

    cycle_cnt  = max_cnt;
    first_pass = 0;
    return 1;

  }

  if (is_persistent) {

    if (--cycle_cnt) {

      _raise(SIGSTOP);

      __afl_area_ptr[0] = 1;
      __afl_prev_loc = 0;

      return 1;

    } else {

      /* When exiting __AFL_LOOP(), make sure that the subsequent code that
         follows the loop is not traced. We do that by pivoting back to the
         dummy output region. */
      // we have judgement in _afl_maybe_log, no need to use dummy area
      // XXX: but which is faster?
      __afl_area_ptr = 0;

    }

  }

  return 0;

}


void init(int argc, char** argv, char** envp)
{
    environment = envp;
    __afl_auto_init();
}
