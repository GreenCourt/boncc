CFLAGS=-std=c11 -g -static -Wall -Wextra -Werror -MMD
OBJ_DIR=obj
TEST_OBJ_DIR=test/obj
TEST_EXE_DIR=test/exe

default: boncc

boncc: $(addprefix $(OBJ_DIR)/, main.o common.o tokenize.o parse.o codegen.o Vector.o type.o node.o)
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

$(TEST_EXE_DIR)/%: $(addprefix $(TEST_OBJ_DIR)/,common.o %.o)
	@mkdir -p $(TEST_EXE_DIR)
	$(CC) -o $@ $^ $(LDFLAGS)

$(TEST_OBJ_DIR)/%.o: test/%.c boncc
	@mkdir -p $(TEST_OBJ_DIR)
	./boncc $< > $(basename $@).s
	$(CC) $(CFLAGS) -c -o $@ $(basename $@).s

$(TEST_OBJ_DIR)/common.o: test/common.c
	@mkdir -p $(TEST_OBJ_DIR)
	$(CC) $(CFLAGS) -c -o $@ $<

$(TEST_EXE_DIR)/Vector: $(TEST_OBJ_DIR)/Vector.o $(OBJ_DIR)/Vector.o
	@mkdir -p $(TEST_EXE_DIR)
	$(CC) -o $@ $^ $(LDFLAGS)

$(TEST_OBJ_DIR)/Vector.o: test/Vector.c
	@mkdir -p $(TEST_OBJ_DIR)
	$(CC) $(CFLAGS) -c -o $@ $<

-include $(OBJ_DIR)/*.d
.PHONY: default test clean fmt
