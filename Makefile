CFLAGS:=-std=c11 -g -static

boncc: boncc.c

test: boncc
	./test.sh

clean:
	rm -f boncc *.o tmp*

.PHONY: test clean
