CC = gcc
CFLAGS = -Wall -Wextra -O2 -std=c11
DEBUG_FLAGS = -g -O0 -DDEBUG -Wall -Wextra -std=c11
INCLUDES = -Iinclude

SRC_DIR = src
BUILD_DIR = build

SRCS = $(wildcard $(SRC_DIR)/*.c)
OBJS = $(patsubst $(SRC_DIR)/%.c,$(BUILD_DIR)/%.o,$(SRCS))
DEPS = $(OBJS:.o=.d)

TARGET = $(BUILD_DIR)/grafos

TEST_DIR = tests
TEST_TARGET = $(BUILD_DIR)/test_csv_parser
TEST_SRCS = $(TEST_DIR)/test_csv_parser.c $(SRC_DIR)/csv_parser.c
TEST_OBJS = $(BUILD_DIR)/test_csv_parser.o $(BUILD_DIR)/csv_parser.o

.PHONY: all debug clean run test

all: CFLAGS += -MMD -MP
all: $(TARGET)

$(TARGET): $(OBJS)
	$(CC) $(CFLAGS) $(INCLUDES) -o $@ $^

$(BUILD_DIR)/%.o: $(SRC_DIR)/%.c | $(BUILD_DIR)
	$(CC) $(CFLAGS) $(INCLUDES) -c $< -o $@

$(BUILD_DIR)/test_csv_parser.o: $(TEST_DIR)/test_csv_parser.c | $(BUILD_DIR)
	$(CC) $(CFLAGS) $(INCLUDES) -c $< -o $@

$(BUILD_DIR):
	mkdir -p $(BUILD_DIR)

-include $(DEPS)

debug: CFLAGS = $(DEBUG_FLAGS)
debug: clean $(TARGET)

run: all
	./$(TARGET)

test: CFLAGS += -MMD -MP
test: $(TEST_OBJS)
	$(CC) $(CFLAGS) $(INCLUDES) -o $(TEST_TARGET) $(TEST_OBJS)
	./$(TEST_TARGET)

clean:
	rm -rf $(BUILD_DIR)
