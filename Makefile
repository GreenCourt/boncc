# TOPDIR must be set here (before including other makefiles)
TOPDIR:=$(abspath $(dir $(lastword $(MAKEFILE_LIST))))

CFLAGS=-std=c11 -g -static -Wall -Wextra -Werror -MMD
LDFLAGS=-znoexecstack
OBJ_DIR=obj
TEST_OBJ_DIR=test/obj
TEST_EXE_DIR=test/exe

DEP=main common tokenizer parser generator preprocessor vector type node map number
TESTS=$(basename $(filter-out common.c func.c,$(notdir $(wildcard test/*.c))))

boncc: $(addprefix $(OBJ_DIR)/,$(addsuffix .o,$(DEP)))
	$(CC) -o $@ $^ $(LDFLAGS)

test: boncc gtest stage1test abitest
all: gtest stage1test abitest stage2test stage3test

clean:
	rm -rf boncc boncc2 boncc3 $(OBJ_DIR) $(TEST_OBJ_DIR) $(TEST_EXE_DIR)

fmt:
	clang-format -i *.c *.h test/*.c

$(OBJ_DIR)/%.o:%.c
	@mkdir -p $(OBJ_DIR)
	$(CC) $(CFLAGS) $(EXTRA_CFLAGS) -c -o $@ $<

-include $(OBJ_DIR)/*.d
.PHONY: all test stage1test stage2test stage3test abitest clean fmt gtest

BONCC_INCLUDE_PATH?=$(TOPDIR)/include
EXTRA_CFLAGS:=
$(OBJ_DIR)/main.o: EXTRA_CFLAGS:=-D BONCC_INCLUDE_PATH=\"$(BONCC_INCLUDE_PATH)\"
$(OBJ_DIR)/main2.o: EXTRA_CFLAGS:=-D BONCC_INCLUDE_PATH=\"$(BONCC_INCLUDE_PATH)\"
$(OBJ_DIR)/main3.s: EXTRA_CFLAGS:=-D BONCC_INCLUDE_PATH=\"$(BONCC_INCLUDE_PATH)\"

TEST_MACRO=-D COMMAND_ARG_OBJ_LIKE_ONE \
	   -D COMMAND_ARG_FUNC_LIKE_ONE\(X,Y,...\) \
	   -D COMMAND_ARG_OBJ_LIKE_EMPTY= \
	   -D COMMAND_ARG_FUNC_LIKE_EMPTY\(X,Y,...\)= \
	   -DCOMMAND_ARG_OBJ_LIKE_ONE2 \
	   -DCOMMAND_ARG_FUNC_LIKE_ONE2\(X\) \
	   -DCOMMAND_ARG_OBJ_LIKE_EMPTY2= \
	   -DCOMMAND_ARG_FUNC_LIKE_EMPTY2\(X\)= \
	   -D COMMAND_ARG_OBJ_LIKE_22=22 \
	   -D COMMAND_ARG_FUNC_LIKE_JOIN\(X,Y\)=X\#\#Y \
	   -DCOMMAND_ARG_OBJ_LIKE_24=24 \
	   -DCOMMAND_ARG_FUNC_LIKE_MINUS\(X\)=-X
$(TEST_EXE_DIR)/gcc_macro: EXTRA_TEST_FLAGS=$(TEST_MACRO)
$(TEST_OBJ_DIR)/macro.o: EXTRA_TEST_FLAGS=$(TEST_MACRO)
$(TEST_OBJ_DIR)/macro2.o: EXTRA_TEST_FLAGS=$(TEST_MACRO)

#########################################
#
# verify test codes by gcc
#
#########################################

gtest: $(addprefix $(TEST_EXE_DIR)/,$(addprefix gcc_,$(TESTS)))
	for i in $^; do echo $$i; $$i || exit $$?; done


$(TEST_OBJ_DIR)/gcc_%.o: test/%.c
	@mkdir -p $(TEST_OBJ_DIR)
	gcc -c -w $(EXTRA_TEST_FLAGS) -o $@ $<

$(TEST_OBJ_DIR)/gcc_call.o: test/func.h
$(TEST_OBJ_DIR)/gcc_func.o: test/func.h

$(TEST_EXE_DIR)/gcc_call: $(TEST_OBJ_DIR)/gcc_func.o
$(TEST_EXE_DIR)/gcc_vector: $(OBJ_DIR)/vector.o
$(TEST_EXE_DIR)/gcc_%: $(TEST_OBJ_DIR)/gcc_%.o $(TEST_OBJ_DIR)/gcc_common.o
	@mkdir -p $(TEST_EXE_DIR)
	gcc -w $(EXTRA_TEST_FLAGS) -o $@ $^


#########################################
#
# ABI compatibility test
#
#########################################
abitest: $(TEST_EXE_DIR)/call_gcc_obj $(TEST_EXE_DIR)/called_by_gcc
	for i in $^; do echo $$i; $$i || exit $$?; done

$(TEST_EXE_DIR)/call_gcc_obj: $(TEST_OBJ_DIR)/gcc_call.o $(TEST_OBJ_DIR)/func.o $(TEST_OBJ_DIR)/common.o
	@mkdir -p $(TEST_EXE_DIR)
	$(CC) -o $@ $^ $(LDFLAGS)

$(TEST_EXE_DIR)/called_by_gcc: $(TEST_OBJ_DIR)/call.o $(TEST_OBJ_DIR)/gcc_func.o $(TEST_OBJ_DIR)/gcc_common.o
	@mkdir -p $(TEST_EXE_DIR)
	$(CC) -o $@ $^ $(LDFLAGS)

#########################################
#
# stage1 test
#
#########################################

stage1test: $(addprefix $(TEST_EXE_DIR)/,$(TESTS))
	for i in $^; do echo $$i; $$i || exit $$?; done

$(TEST_EXE_DIR)/call: $(TEST_OBJ_DIR)/func.o
$(TEST_EXE_DIR)/vector: $(OBJ_DIR)/vector.o
$(TEST_EXE_DIR)/%: $(addprefix $(TEST_OBJ_DIR)/,%.o common.o)
	@mkdir -p $(TEST_EXE_DIR)
	$(CC) -o $@ $^ $(LDFLAGS)

$(TEST_OBJ_DIR)/%.o: test/%.c boncc
	@mkdir -p $(TEST_OBJ_DIR)
	./boncc $(EXTRA_TEST_FLAGS) $< -o $(basename $@).s
	as -g -o $@ $(basename $@).s

$(TEST_OBJ_DIR)/call.o: test/func.h
$(TEST_OBJ_DIR)/func.o: test/func.h

#########################################
#
# stage2 test
#
#########################################

boncc2: $(addprefix $(OBJ_DIR)/,$(addsuffix 2.o,$(DEP)))
	$(CC) -o $@ $^ $(LDFLAGS)

$(OBJ_DIR)/%2.o: %.c boncc
	./boncc $(EXTRA_CFLAGS) $< -o $(basename $@).s
	as -g $(basename $@).s -o $@

stage2test: $(addprefix $(TEST_EXE_DIR)/,$(addsuffix 2,$(TESTS)))
	for i in $^; do echo $$i; $$i || exit $$?; done

$(TEST_EXE_DIR)/call2: $(TEST_OBJ_DIR)/func2.o
$(TEST_EXE_DIR)/vector2: $(OBJ_DIR)/vector2.o
$(TEST_EXE_DIR)/%2: $(addprefix $(TEST_OBJ_DIR)/,%2.o common2.o)
	@mkdir -p $(TEST_EXE_DIR)
	$(CC) -o $@ $^ $(LDFLAGS)

$(TEST_OBJ_DIR)/%2.o: test/%.c boncc2
	@mkdir -p $(TEST_OBJ_DIR)
	./boncc2 $(EXTRA_TEST_FLAGS) $< -o $(basename $@).s
	as -g -o $@ $(basename $@).s

$(TEST_OBJ_DIR)/call2.o: test/func.h
$(TEST_OBJ_DIR)/func2.o: test/func.h

#########################################
#
# stage3 test
#
#########################################

stage3test: $(addprefix $(OBJ_DIR)/,$(addsuffix 3.s,$(DEP)))
	for i in $(DEP); do diff -sq $(OBJ_DIR)/$${i}2.s $(OBJ_DIR)/$${i}3.s || exit $$?; done

$(OBJ_DIR)/%3.s: %.c boncc2
	./boncc2 $(EXTRA_CFLAGS) $< -o $@
