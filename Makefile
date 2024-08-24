ifeq (run,$(firstword $(MAKECMDGOALS)))
  RUN_ARGS := $(wordlist 2,$(words $(MAKECMDGOALS)),$(MAKECMDGOALS))
  $(eval $(RUN_ARGS):;@:)
endif

SOURCE_PATH := src
BUILD_PATH := build
TEST_PATH := test
UNIT_TEST_PATH := $(TEST_PATH)/unit
VENDOR_PATH := vendor
UNITY_PATH := $(VENDOR_PATH)/Unity/src
BUILD_OBJECTS_PATH := $(BUILD_PATH)/objects
BUILD_DEPENDS_PATH := $(BUILD_PATH)/depends
BUILD_RESULTS_PATH := $(BUILD_PATH)/results

SOURCES := $(wildcard $(SOURCE_PATH)/*.c)
OBJECTS := $(patsubst $(SOURCE_PATH)/%.c,$(BUILD_OBJECTS_PATH)/%.o,$(SOURCES))
DEPENDS := $(patsubst $(SOURCE_PATH)/%.c,$(BUILD_DEPENDS_PATH)/%.d,$(SOURCES))
TARGET := $(BUILD_PATH)/clox

UNIT_TEST_SOURCES := $(wildcard $(UNIT_TEST_PATH)/*.c)
UNIT_TEST_OBJECTS := $(patsubst $(UNIT_TEST_PATH)/%.c,$(BUILD_OBJECTS_PATH)/%.o,$(UNIT_TEST_SOURCES))
UNIT_TEST_TARGETS := $(patsubst $(UNIT_TEST_PATH)/test_%.c,$(BUILD_PATH)/test_%.out,$(UNIT_TEST_SOURCES))
UNIT_TEST_RESULTS := $(patsubst $(UNIT_TEST_PATH)/test_%.c,$(BUILD_RESULTS_PATH)/test_%.txt,$(UNIT_TEST_SOURCES))
DEPENDS += $(patsubst $(UNIT_TEST_PATH)/%.c,$(BUILD_DEPENDS_PATH)/%.d,$(UNIT_TEST_SOURCES))

UNITY_SOURCES := $(wildcard $(UNITY_PATH)/*.c)
UNITY_OBJECTS := $(patsubst $(UNITY_PATH)/%.c,$(BUILD_OBJECTS_PATH)/%.o,$(UNITY_SOURCES))
DEPENDS += $(patsubst $(UNITY_PATH)/%.c,$(BUILD_DEPENDS_PATH)/%.d,$(UNITY_SOURCES))

C_STD := c99
COMPILE := clang -std=${C_STD}
LINK := clang -std=${C_STD}
WARNINGS := -Wall -Wextra
ASAN := -fsanitize=address -fno-omit-frame-pointer
LOG_DEBUG := -g -DDEBUG_TRACE_EXECUTION -DDEBUG_ALLOCATIONS -DDEFAULT_LOG_LEVEL=LOG_LEVEL_DEBUG
INCLUDES := -I$(SOURCE_PATH) -I/opt/homebrew/opt/llvm/include
COMPILE_FLAGS := $(INCLUDES) $(WARNINGS) $(LOG_DEBUG) -g
UNIT_TEST_COMPILE_FLAGS := $(COMPILE_FLAGS) -I$(UNIT_TEST_PATH)/include -I$(UNITY_PATH) -DTEST
DEPENDS_FLAGS = -MT $@ -MMD -MP -MF $(BUILD_DEPENDS_PATH)/$*.d
LINK_FLAGS :=

.PRECIOUS: $(BUILD_PATH)/test_%.out
.PRECIOUS: $(BUILD_DEPENDS_PATH)/%.d
.PRECIOUS: $(BUILD_OBJECTS_PATH)/%.o
.PRECIOUS: $(PATH_BUILD_RESULTS)/%.txt

.PHONY: all target run test unit_test clean

all: target test

run: $(TARGET)
	@echo "=> Running target ($<) $(RUN_ARGS)"
	./$< $(RUN_ARGS)

target: $(TARGET)

test: unit_test

unit_test: $(UNIT_TEST_RESULTS)
	@echo "=> Unit Test Results"
	@PASS_RESULTS=$$(find $(BUILD_RESULTS_PATH) -type f -name '*.txt' -exec grep -s PASS {} +); \
	if [ ! -z "$$PASS_RESULTS" ]; then \
		while IFS= read -r line; do printf "( \033[1;32mOK\033[0m ) $$line\n"; done <<< "$$PASS_RESULTS"; \
	fi
	@IGNORE_RESULTS=$$(find $(BUILD_RESULTS_PATH) -type f -name '*.txt' -exec grep -s IGNORE {} +); \
	if [ ! -z "$$IGNORE_RESULTS" ]; then \
		while IFS= read -r line; do printf "( \033[1;33mSKIP\033[0m ) $$line\n"; done <<< "$$IGNORE_RESULTS"; \
	fi
	@FAIL_RESULTS=$$(find $(BUILD_RESULTS_PATH) -type f -name '*.txt' -exec grep -s FAIL {} +); \
	if [ ! -z "$$FAIL_RESULTS" ]; then \
		while IFS= read -r line; do printf "( \033[1;31mFAIL\033[0m ) $$line\n"; done <<< "$$FAIL_RESULTS"; \
	fi

clean:
	rm -f $(TARGET)
	rm -f $(BUILD_PATH)/*.out
	rm -f $(BUILD_OBJECTS_PATH)/*.o
	rm -f $(BUILD_RESULTS_PATH)/*.txt
	rm -f $(BUILD_DEPENDS_PATH)/*.d

-include $(OBJECTS:.o=.d)
-include $(UNIT_TEST_OBJECTS:.o=.d)
-include $(UNITY_OBJECTS:.o=.d)

$(BUILD_PATH)/clox: $(OBJECTS) | $(BUILD_PATH)
	@echo "=> Building target ($@)"
	$(LINK) $(LINK_FLAGS) -o $@ $^

$(BUILD_PATH)/test_%.out: $(BUILD_OBJECTS_PATH)/test_%.o $(BUILD_OBJECTS_PATH)/helpers.o  $(UNITY_OBJECTS) $(filter-out $(BUILD_OBJECTS_PATH)/main.o,$(OBJECTS)) | $(BUILD_OBJECTS_PATH) $(BUILD_PATH)
	@echo "=> Building test target ($@)"
	$(LINK) $(LINK_FLAGS) -o $@ $^

$(BUILD_OBJECTS_PATH)/%.o:: $(SOURCE_PATH)/%.c | $(BUILD_OBJECTS_PATH) $(BUILD_DEPENDS_PATH)
	$(COMPILE) -c $(COMPILE_FLAGS) $(DEPENDS_FLAGS) -o $@ $<

$(BUILD_OBJECTS_PATH)/%.o:: $(UNIT_TEST_PATH)/%.c | $(BUILD_OBJECTS_PATH) $(BUILD_DEPENDS_PATH)
	$(COMPILE) -c $(UNIT_TEST_COMPILE_FLAGS) $(DEPENDS_FLAGS) -o $@ $<

$(BUILD_OBJECTS_PATH)/%.o:: $(UNITY_PATH)/%.c $(UNITY_PATH)/%.h | $(BUILD_OBJECTS_PATH) $(BUILD_DEPENDS_PATH)
	$(COMPILE) -c $(UNIT_TEST_COMPILE_FLAGS) $(DEPENDS_FLAGS) -o $@ $<

$(BUILD_RESULTS_PATH)/test_%.txt: $(BUILD_PATH)/test_%.out | $(BUILD_RESULTS_PATH)
	@echo "=> Executing test target ($@)"
	-./$< > $@ 2>&1

$(DEPENDS):

$(BUILD_PATH) $(BUILD_OBJECTS_PATH) $(BUILD_DEPENDS_PATH) $(BUILD_RESULTS_PATH):
	mkdir -p $@

