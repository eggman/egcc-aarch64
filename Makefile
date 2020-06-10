CC=aarch64-linux-gnu-gcc
CFLAGS=-std=c11 -g  -fno-common
LDFLAGS=-static
SRCS=$(wildcard *.c)
OBJS=$(SRCS:.c=.o)

egcc: $(OBJS)
	$(CC) -o $@ $(OBJS) $(LDFLAGS)

$(OBJS): egcc.h

test: egcc
	./test.sh

clean:
	rm -f egcc *.o  tmp*

.PHONY: test clean
