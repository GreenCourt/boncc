# TOPDIR must be set here (before including other makefiles)
TOPDIR:=$(abspath $(dir $(lastword $(MAKEFILE_LIST))))

CFLAGS=-std=c11 -g -static -Wall -Wextra -Werror -MMD
LDFLAGS=-znoexecstack

DEP=main common tokenizer parser generator preprocessor vector type node map number builtin_headers
TESTS=$(basename $(filter-out common.c func.c,$(notdir $(wildcard test/*.c))))

boncc: $(addprefix build/obj/,$(addsuffix .o,$(DEP)))
	$(CC) -o $@ $^ $(LDFLAGS)

test: boncc gtest test1 test2 test3

clean:
	rm -rf boncc boncc2 build

fmt:
	clang-format -i *.c *.h test/*.c test/*.h include/*.h

build/obj/%.o:%.c
	@mkdir -p $(dir $@)
	$(CC) $(CFLAGS) -c -o $@ $<

build/obj/builtin_headers.o: build/obj/embed $(wildcard include/*.h)
	$< $(wordlist 2,$(words $^),$(^)) | $(CC) -xc -c -o $@ - $(CFLAGS)
build/obj/embed: embed.c
	@mkdir -p $(dir $@)
	$(CC) -o $@ $^ $(CFLAGS) $(LDFLAGS)

-include build/*.d build/*/*.d build/*/*/*.d
.PHONY: test test1 test2 test3 clean fmt gtest

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
build/test/gcc/macro.o: EXTRA_TEST_FLAGS=$(TEST_MACRO)
build/test/1/macro.o: EXTRA_TEST_FLAGS=$(TEST_MACRO)
build/test/2/macro.o: EXTRA_TEST_FLAGS=$(TEST_MACRO)

#########################################
#
# verify test codes by gcc
#
#########################################

gtest: $(addprefix build/test/gcc/,$(TESTS))
	for i in $^; do echo $$i; $$i || exit $$?; done

build/test/gcc/%.o: test/%.c
	@mkdir -p $(dir $@)
	gcc -c -w -MMD $(EXTRA_TEST_FLAGS) -o $@ $<

build/test/gcc/call: build/test/gcc/func.o
build/test/gcc/vector: build/obj/vector.o
build/test/gcc/%: build/test/gcc/%.o build/test/gcc/common.o
	$(CC) -o $@ $^ $(LDFLAGS)


#########################################
#
# ABI compatibility test
#
#########################################
build/test/%/called_by_gcc: build/test/gcc/call.o build/test/%/func.o build/test/%/common.o
	$(CC) -o $@ $^ $(LDFLAGS)

build/test/%/call_gcc_obj: build/test/%/call.o build/test/gcc/func.o build/test/gcc/common.o
	$(CC) -o $@ $^ $(LDFLAGS)

#########################################
#
# stage1 test
#
#########################################

test1: $(addprefix build/test/1/,$(TESTS) called_by_gcc call_gcc_obj)
	for i in $^; do echo $$i; $$i || exit $$?; done

build/test/1/call: build/test/1/func.o
build/test/1/vector: build/obj/vector.o
build/test/1/%: $(addprefix build/test/1/,%.o common.o)
	$(CC) -o $@ $^ $(LDFLAGS)

build/test/1/%.o: test/%.c boncc
	@mkdir -p $(dir $@)
	./boncc -MMD -c $(EXTRA_TEST_FLAGS) $< -o $@

#########################################
#
# stage2 test
#
#########################################

boncc2: $(addprefix build/obj2/,$(addsuffix .o,$(DEP)))
	$(CC) -o $@ $^ $(LDFLAGS)

build/obj2/%.o: %.c boncc
	@mkdir -p $(dir $@)
	./boncc -MMD -c $< -o $@

build/obj2/builtin_headers.o: boncc build/obj2/embed $(wildcard include/*.h)
	$(word 2,$^) $(wordlist 3,$(words $^),$^) | ./boncc -MMD -c -o $@ -
build/obj2/embed: build/obj2/embed.s
	$(CC) -o $@ $< $(LDFLAGS)
build/obj2/embed.s: embed.c boncc
	@mkdir -p $(dir $@)
	./boncc -MMD -S $< -o $@

test2: $(addprefix build/test/2/,$(TESTS) call_gcc_obj called_by_gcc)
	for i in $^; do echo $$i; $$i || exit $$?; done

build/test/2/call: build/test/2/func.o
build/test/2/vector: build/obj2/vector.o
build/test/2/%: $(addprefix build/test/2/,%.o common.o)
	$(CC) -o $@ $^ $(LDFLAGS)

build/test/2/%.o: test/%.c boncc2
	@mkdir -p $(dir $@)
	./boncc2 -MMD -c $(EXTRA_TEST_FLAGS) $< -o $@

#########################################
#
# stage3 test
#
#########################################

test3: $(addprefix build/obj3/,$(addsuffix .s,$(DEP)) embed.s)
	for i in $^; do diff -sq "$${i}" "build/obj2/$${i##*/}" || exit $$?; done

build/obj3/%.s: %.c boncc2
	@mkdir -p $(dir $@)
	./boncc2 -MMD -S $< -o $@

build/obj3/builtin_headers.s: boncc2 build/obj3/embed $(wildcard include/*.h)
	$(word 2,$^) $(wordlist 3,$(words $^),$^) | ./boncc2 -MMD -S -o $@ -
build/obj3/embed: build/obj3/embed.s
	$(CC) -o $@ $< $(LDFLAGS)
build/obj3/embed.s: embed.c boncc2
	@mkdir -p $(dir $@)
	./boncc2 -MMD -S $< -o $@
