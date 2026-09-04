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

.PHONY: all debug clean run

all: CFLAGS += -MMD -MP
all: $(TARGET)

$(TARGET): $(OBJS)
	$(CC) $(CFLAGS) $(INCLUDES) -o $@ $^

$(BUILD_DIR)/%.o: $(SRC_DIR)/%.c | $(BUILD_DIR)
	$(CC) $(CFLAGS) $(INCLUDES) -c $< -o $@

$(BUILD_DIR):
	mkdir -p $(BUILD_DIR)

-include $(DEPS)

debug: CFLAGS = $(DEBUG_FLAGS)
debug: clean $(TARGET)

run: all
	./$(TARGET)

clean:
	rm -rf $(BUILD_DIR)
