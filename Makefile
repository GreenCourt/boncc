CFLAGS:=-std=c11 -g -static -Wall -Wextra
SRCS=$(wildcard *.c)
OBJS=$(SRCS:.c=.o)

boncc: $(OBJS)
	$(CC) -o $@ $^ $(LDFLAGS)

$(OBJS): boncc.h

test: boncc
	./test.sh

clean:
	rm -f boncc *.o tmp*

fmt:
	clang-format -i *.c *.h

.PHONY: test clean fmt
