CXX := g++
CC := gcc
LD := g++
TARGET_NAME := sclp

BUILD_DIR := build
SRC_DIR := src

TEST_DIR := tests

TARGET_EXEC := ./$(TARGET_NAME)

REPODIR := $(shell pwd)
TESTLOCK := $(REPODIR)/$(BUILD_DIR)/.test_lock

FLEX_SRCS := $(shell find $(SRC_DIR) -name '*.l')
BISON_SRCS := $(shell find $(SRC_DIR) -name '*.y')
NORMAL_SRCS := $(shell find $(SRC_DIR) -name '*.c' -or -name '*.cc')

FLEX_C_SRCS := $(FLEX_SRCS:%.l=$(BUILD_DIR)/%.l.c)
BISON_C_SRCS := $(BISON_SRCS:%.y=$(BUILD_DIR)/%.y.tab.c)
BISON_C_HDRS := $(BISON_SRCS:%.y=$(BUILD_DIR)/%.y.tab.h)
FLEX_OBJS := $(FLEX_C_SRCS:%.c=%.o)
BISON_OBJS := $(BISON_C_SRCS:%.c=%.o)

SRCS := $(NORMAL_SRCS) $(FLEX_C_SRCS) $(BISON_C_SRCS)
OBJS := $(NORMAL_SRCS:%=$(BUILD_DIR)/%.o) $(FLEX_OBJS) $(BISON_OBJS)

DEPS := $(OBJS:.o=.d)

INC_FLAGS := -I$(SRC_DIR) -I$(BUILD_DIR)/$(SRC_DIR)

BISON_FLAGS := -Wconflicts-sr -Wcounterexamples -d
CXX_FLAGS := -Wall -Wpedantic -Werror $(INC_FLAGS) -g -rdynamic -MMD -MP
CC_FLAGS := $(INC_FLAGS) -g -MMD -MP
LD_FLAGS :=
LIB_FLAGS := -ly -ll

all: $(TARGET_EXEC)

cleantest:
	@$(RM) -r "$(TESTLOCK)"

test: $(TESTLOCK)

$(TESTLOCK): $(TARGET_EXEC)
	@echo "At least write THIS yourself lmao"

$(TARGET_EXEC): $(OBJS)
	@mkdir -p $(dir $@)
	$(LD) $(LD_FLAGS) -o $@ $^ $(LIB_FLAGS)

$(BUILD_DIR)/%.cc.o: %.cc
	@mkdir -p $(dir $@)
	$(CXX) -c $(CXX_FLAGS) -o $@ $<

$(BUILD_DIR)/%.l.o: $(BUILD_DIR)/%.l.c
	@mkdir -p $(dir $@)
	$(CC) -c $(CC_FLAGS) -o $@ $<

$(BUILD_DIR)/%.y.tab.o: $(BUILD_DIR)/%.y.tab.c
	@mkdir -p $(dir $@)
	$(CC) -c $(CC_FLAGS) -o $@ $<

$(BUILD_DIR)/%.c.o: %.c
	@mkdir -p $(dir $@)
	$(CC) -c $(CC_FLAGS) -o $@ $<

$(BUILD_DIR)/%.l.c: %.l $(BISON_C_HDRS)
	@mkdir -p $(dir $@)
	flex -o $@ $<

$(BUILD_DIR)/%.y.tab.c $(BUILD_DIR)/%.y.tab.h: %.y
	@mkdir -p $(dir $@)
	bison $(BISON_FLAGS) -b $(BUILD_DIR)/$*.y $<

clean:
	@$(RM) -r $(BUILD_DIR) $(TARGET_EXEC)
	@$(RM) -r $(shell find . -name '*.toks' -or -name '*.ast' -or -name '*.tac' -or -name '*.rtl' -or -name '*.spim' -or -name '*.log')

.PHONY: all clean test cleantest

-include $(DEPS)
