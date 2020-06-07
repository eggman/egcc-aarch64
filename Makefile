CC=aarch64-linux-gnu-gcc
CFLAGS=-std=c11 -g  -fno-common
LDFLAGS=-static

egcc: main.o
	$(CC) -o $@ $? $(LDFLAGS)

test: egcc
	./test.sh

clean:
	rm -f egcc *.o  tmp*

.PHONY: test clean
