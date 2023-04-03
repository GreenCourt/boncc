CFLAGS=-std=c11 -g -static -Wall -Wextra -Werror -MMD
OBJ_DIR=obj
TEST_OBJ_DIR=test/obj
TEST_EXE_DIR=test/exe

DEP=main common tokenizer parser generator preprocessor vector type node map
TESTS=$(basename $(filter-out common.c,$(notdir $(wildcard test/*.c))))

boncc: $(addprefix $(OBJ_DIR)/,$(addsuffix .o,$(DEP)))
	$(CC) -o $@ $^ $(LDFLAGS)

test: gtest stage1test
all: fmt gtest stage1test stage2test stage3test

clean:
	rm -rf boncc boncc2 boncc3 $(OBJ_DIR) $(TEST_OBJ_DIR) $(TEST_EXE_DIR)

fmt:
	clang-format -i *.c *.h test/*.c

$(OBJ_DIR)/%.o:%.c
	@mkdir -p $(OBJ_DIR)
	$(CC) $(CFLAGS) -c -o $@ $<

-include $(OBJ_DIR)/*.d
.PHONY: all test stage1test stage2test stage3test clean fmt gtest

#########################################
#
# verify test codes by gcc
#
#########################################

gtest: $(addprefix $(TEST_EXE_DIR)/,$(addprefix gcc_,$(TESTS)))
	for i in $^; do echo $$i; $$i || exit $$?; done

$(TEST_EXE_DIR)/gcc_vector: vector.c
$(TEST_EXE_DIR)/gcc_%: test/%.c test/common.c
	@mkdir -p $(TEST_EXE_DIR)
	gcc -w -o $@ $^

#########################################
#
# stage1 test
#
#########################################

stage1test: $(addprefix $(TEST_EXE_DIR)/,$(TESTS))
	for i in $^; do echo $$i; $$i || exit $$?; done

$(TEST_EXE_DIR)/vector: $(OBJ_DIR)/vector.o
$(TEST_EXE_DIR)/%: $(addprefix $(TEST_OBJ_DIR)/,%.o common.o)
	@mkdir -p $(TEST_EXE_DIR)
	$(CC) -o $@ $^ $(LDFLAGS)

$(TEST_OBJ_DIR)/%.o: test/%.c boncc
	@mkdir -p $(TEST_OBJ_DIR)
	./boncc $< -o $(basename $@).s
	as -g -o $@ $(basename $@).s

#########################################
#
# stage2 test
#
#########################################

boncc2: $(addprefix $(OBJ_DIR)/,$(addsuffix 2.o,$(DEP)))
	$(CC) -o $@ $^ $(LDFLAGS)

$(OBJ_DIR)/%2.o: %.c boncc
	./boncc $< -o $(basename $@).s
	as -g $(basename $@).s -o $@

stage2test: $(addprefix $(TEST_EXE_DIR)/,$(addsuffix 2,$(TESTS)))
	for i in $^; do echo $$i; $$i || exit $$?; done

$(TEST_EXE_DIR)/vector2: $(OBJ_DIR)/vector2.o
$(TEST_EXE_DIR)/%2: $(addprefix $(TEST_OBJ_DIR)/,%2.o common2.o)
	@mkdir -p $(TEST_EXE_DIR)
	$(CC) -o $@ $^ $(LDFLAGS)

$(TEST_OBJ_DIR)/%2.o: test/%.c boncc2
	@mkdir -p $(TEST_OBJ_DIR)
	./boncc2 $< -o $(basename $@).s
	as -g -o $@ $(basename $@).s

#########################################
#
# stage3 test
#
#########################################

stage3test: $(addprefix $(OBJ_DIR)/,$(addsuffix 3.s,$(DEP)))
	for i in $(DEP); do diff -sq $(OBJ_DIR)/$${i}2.s $(OBJ_DIR)/$${i}3.s || exit $$?; done

$(OBJ_DIR)/%3.s: %.c boncc2
	./boncc2 $< -o $@
