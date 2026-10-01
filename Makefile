CC = gcc
CFLAGS = -Wall -Wextra -O2 -std=c11
DEBUG_FLAGS = -g -O0 -DDEBUG -Wall -Wextra -std=c11
INCLUDES = -Iinclude

# GetProcessMemoryInfo (bench.c) fica em psapi no Windows
ifeq ($(OS),Windows_NT)
LDLIBS = -lpsapi
endif

SRC_DIR = src
BUILD_DIR = build

SRCS = $(wildcard $(SRC_DIR)/*.c)
OBJS = $(patsubst $(SRC_DIR)/%.c,$(BUILD_DIR)/%.o,$(SRCS))
DEPS = $(OBJS:.o=.d)

TARGET = $(BUILD_DIR)/grafos

TEST_DIR = tests
TEST_SRCS = $(wildcard $(TEST_DIR)/test_*.c)
TEST_BINS = $(patsubst $(TEST_DIR)/%.c,$(BUILD_DIR)/%,$(TEST_SRCS))
LIB_OBJS = $(filter-out $(BUILD_DIR)/main.o,$(OBJS))

.PHONY: all debug clean run test

all: CFLAGS += -MMD -MP
all: $(TARGET)

$(TARGET): $(OBJS)
	$(CC) $(CFLAGS) $(INCLUDES) -o $@ $^ $(LDLIBS)

$(BUILD_DIR)/%.o: $(SRC_DIR)/%.c | $(BUILD_DIR)
	$(CC) $(CFLAGS) $(INCLUDES) -c $< -o $@

# Cada tests/test_*.c vira um executável ligado a todos os módulos (menos main.o)
$(BUILD_DIR)/test_%: $(TEST_DIR)/test_%.c $(LIB_OBJS) | $(BUILD_DIR)
	$(CC) $(CFLAGS) $(INCLUDES) -o $@ $< $(LIB_OBJS) $(LDLIBS)

$(BUILD_DIR):
	mkdir -p $(BUILD_DIR)

-include $(DEPS)

debug: CFLAGS = $(DEBUG_FLAGS)
debug: clean $(TARGET)

run: all
	./$(TARGET)

test: CFLAGS += -MMD -MP
test: $(TEST_BINS)
	@for t in $(TEST_BINS); do echo "== $$t"; ./$$t || exit 1; done

clean:
	rm -rf $(BUILD_DIR)
