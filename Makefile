CFLAGS=-std=c11 -g -static -Wall -Wextra -Werror -MMD
OBJ_DIR=obj
TEST_OBJ_DIR=test/obj
TEST_EXE_DIR=test/exe

DEP=main common tokenizer parser generator preprocessor vector type node map
TESTS=$(basename $(filter-out common.c,$(notdir $(wildcard test/*.c))))

boncc: $(addprefix $(OBJ_DIR)/,$(addsuffix .o,$(DEP)))
	$(CC) -o $@ $^ $(LDFLAGS)

test: gtest stage1test stage2test stage3test

clean:
	rm -rf boncc boncc2 boncc3 $(OBJ_DIR) $(TEST_OBJ_DIR) $(TEST_EXE_DIR)

fmt:
	clang-format -i *.c *.h test/*.c

$(OBJ_DIR)/%.o:%.c
	@mkdir -p $(OBJ_DIR)
	$(CC) $(CFLAGS) -c -o $@ $<

-include $(OBJ_DIR)/*.d
.PHONY: test stage1test stage2test stage3test clean fmt gtest

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

$(TEST_EXE_DIR)/%: $(addprefix $(TEST_OBJ_DIR)/,%.o common.o)
	@mkdir -p $(TEST_EXE_DIR)
	$(CC) -o $@ $^ $(LDFLAGS)

$(TEST_OBJ_DIR)/%.o: test/%.c boncc
	@mkdir -p $(TEST_OBJ_DIR)
	./boncc $< -o $(basename $@).s
	as -g -o $@ $(basename $@).s

$(TEST_EXE_DIR)/vector: $(TEST_OBJ_DIR)/vector.o $(OBJ_DIR)/vector.o
	@mkdir -p $(TEST_EXE_DIR)
	$(CC) -o $@ $^ $(LDFLAGS)

$(TEST_OBJ_DIR)/vector.o: test/vector.c
	@mkdir -p $(TEST_OBJ_DIR)
	$(CC) $(CFLAGS) -c -o $@ $<

#########################################
#
# stage2 test
#
#########################################

$(OBJ_DIR)/%.pp.c:%.c
	$(CC) -DNDEBUG -DBONCC -E -P $< -o $@

boncc2: $(addprefix $(OBJ_DIR)/,$(addsuffix 2.o,$(DEP)))
	$(CC) -o $@ $^ $(LDFLAGS)
	strip -s $@

$(OBJ_DIR)/%2.o:$(OBJ_DIR)/%.pp.c boncc
	./boncc $< -o $(basename $@).s
	as $(basename $@).s -o $@

stage2test: $(addprefix $(TEST_EXE_DIR)/,$(addsuffix 2,$(filter-out vector,$(TESTS))))
	for i in $^; do echo $$i; $$i || exit $$?; done

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

boncc3: $(addprefix $(OBJ_DIR)/,$(addsuffix 3.o,$(DEP)))
	$(CC) -o $@ $^ $(LDFLAGS)
	strip -s $@

$(OBJ_DIR)/%3.o:$(OBJ_DIR)/%.pp.c boncc2
	./boncc2 $< -o $(basename $@).s
	as $(basename $@).s -o $@

stage3test: $(addprefix $(TEST_EXE_DIR)/,$(addsuffix 3,$(filter-out vector,$(TESTS))))
	for i in $^; do echo $$i; $$i || exit $$?; done

$(TEST_EXE_DIR)/%3: $(addprefix $(TEST_OBJ_DIR)/,%3.o common3.o)
	@mkdir -p $(TEST_EXE_DIR)
	$(CC) -o $@ $^ $(LDFLAGS)

$(TEST_OBJ_DIR)/%3.o: test/%.c boncc3
	@mkdir -p $(TEST_OBJ_DIR)
	./boncc3 $< -o $(basename $@).s
	as -g -o $@ $(basename $@).s
