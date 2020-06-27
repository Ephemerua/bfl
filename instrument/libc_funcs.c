#include "libc_funcs.h"
#include "syscall.h"

int _strncmp(char*a, char*b, size_t len)
{
    while(len){
        if (*a!=*b)
            return 1;
        a++;
        b++;
        len--;
    }
    return 0;
}

size_t _strlen(char* str)
{
    size_t result = 0;
    while(*str)
    {
        result++;
        str++;
    }
    return result;
}


void _abort()
{
    int *p = 0;
    *p= 0xdead;
}

char* _getenv(char* name, char** envp){
    char *result;
    for(char**p = envp; p && *p; p++)
    {
    if (!_strncmp(*p, name, _strlen(name)))
        {
        result = *p + _strlen(name);
        if (*result == '=')
            return result+1;
        }
    }
    return NULL;
}

int64_t _atol(unsigned char* p)
{
    int64_t result = 0;
    unsigned char *end = p;
    while(*end){
        end++;
    }
    while(end>p){
        result *= 10;
        if(*p>'9'||*p<'0')
            _abort();
        result += *p-'0';
        p++;
    }
    return result;
}

void * _memset(void *b, int c, size_t len)
{
    len--;
    while(len >= 0)
    {
        *(char*)(b + len) = (char)c;
        len--;
    }
    return b;
}


int _raise(int sig)
{
    int pid = bfl_getpid();
    return bfl_kill(pid, sig);
}