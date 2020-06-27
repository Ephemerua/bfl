#include <stdint.h>
#include <stddef.h>

int _strncmp(char*a, char*b, size_t len);
size_t _strlen(char* str);
void _abort();
char* _getenv(char* name, char** envp);
int64_t _atol(unsigned char* p);
void * _memset(void *b, int c, size_t len);

