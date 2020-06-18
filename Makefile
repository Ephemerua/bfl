CC = gcc
VPATH = ./instrument/
OBJS = bfl.o libc_funcs.o
ASM_OBJS = syscall.o
ASM = syscall.S
CFLAGS = -fno-stack-protector -fpie -O2 -Wall
LDFLAGS = -pie -nostdlib -z max-page-size=4096 \
    --export-dynamic --entry=0x0 --strip-all
PROG = bfl

all: $(PROG)

$(PROG): $(OBJS) $(ASM_OBJS)
	ld $(LDFLAGS) $(OBJS) $(ASM_OBJS) -o $@


$(OBJS): %.o: %.c
	$(CC) -c $(CFLAGS) $< -o $@

$(ASM_OBJS):
	$(CC) -c $(VPATH)/$(ASM) -o $@

bfl.o: syscall.h libc_funcs.h
libc_funcs.o: syscall.h libc_funcs.h


.PHONY : clean
clean:
	rm *.o
	rm bfl
