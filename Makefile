CFLAGS=-std=c11 -g -static -Wall -Wextra -MMD
OBJ_DIR=obj

default: boncc test_runner

$(OBJ_DIR)/%.o:%.c
	@mkdir -p $(OBJ_DIR)
	$(CC) $(CFLAGS) -c -o $@ $<

boncc: $(addprefix $(OBJ_DIR)/, main.o common.o tokenize.o parse.o codegen.o Vector.o)
	$(CC) -o $@ $^ $(LDFLAGS)

test_runner: $(addprefix $(OBJ_DIR)/, test_runner.o Vector.o)
	$(CC) -o $@ $^ $(LDFLAGS)

test: boncc test_runner
	./test.sh
	./test_runner

clean:
	rm -rf boncc test_runner tmp* $(OBJ_DIR)

fmt:
	clang-format -i *.c *.h

-include $(OBJ_DIR)/*.d

.PHONY: default test clean fmt
