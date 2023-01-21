CFLAGS=-std=c11 -g -static -Wall -Wextra -Werror -MMD
OBJ_DIR=obj
TEST_OBJ_DIR=test/obj
TEST_EXE_DIR=test/exe

boncc: $(addprefix $(OBJ_DIR)/, main.o common.o tokenizer.o parser.o generator.o vector.o type.o node.o map.o)
	$(CC) -o $@ $^ $(LDFLAGS)

test: $(addprefix $(TEST_EXE_DIR)/,$(basename $(filter-out common.c,$(notdir $(wildcard test/*.c)))))
	for i in $^; do echo $$i; $$i || exit $$?; done

clean:
	rm -rf boncc $(OBJ_DIR) $(TEST_OBJ_DIR) $(TEST_EXE_DIR)

fmt:
	clang-format -i *.c *.h test/*.c

$(OBJ_DIR)/%.o:%.c
	@mkdir -p $(OBJ_DIR)
	$(CC) $(CFLAGS) -c -o $@ $<

$(TEST_OBJ_DIR)/std.o:
	@mkdir -p $(TEST_OBJ_DIR)
	printf "#include <stdio.h>\n#include <stdlib.h>\n" | cc -c -x c - -o $@

$(TEST_EXE_DIR)/%: $(addprefix $(TEST_OBJ_DIR)/,%.o common.o std.o)
	@mkdir -p $(TEST_EXE_DIR)
	$(CC) -o $@ $^ $(LDFLAGS)

$(TEST_OBJ_DIR)/%.o: test/%.c boncc
	@mkdir -p $(TEST_OBJ_DIR)
	./boncc $< -o $(basename $@).s
	$(CC) $(CFLAGS) -c -o $@ $(basename $@).s

$(TEST_EXE_DIR)/vector: $(TEST_OBJ_DIR)/vector.o $(OBJ_DIR)/vector.o
	@mkdir -p $(TEST_EXE_DIR)
	$(CC) -o $@ $^ $(LDFLAGS)

$(TEST_OBJ_DIR)/vector.o: test/vector.c
	@mkdir -p $(TEST_OBJ_DIR)
	$(CC) $(CFLAGS) -c -o $@ $<

gtest: $(addprefix $(TEST_EXE_DIR)/,$(addprefix gcc_,$(basename $(filter-out common.c,$(notdir $(wildcard test/*.c))))))
	for i in $^; do echo $$i; $$i || exit $$?; done

$(TEST_EXE_DIR)/gcc_vector: vector.c
$(TEST_EXE_DIR)/gcc_%: test/%.c test/common.c
	@mkdir -p $(TEST_EXE_DIR)
	gcc -w -o $@ $^


-include $(OBJ_DIR)/*.d
.PHONY: test clean fmt gtest
