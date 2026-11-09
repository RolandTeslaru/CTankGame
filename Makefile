CC      := gcc
CFLAGS  := -std=c11 -Wall -Wextra -g -Iinclude -D_DEFAULT_SOURCE -pthread
LDLIBS  := -lncurses -pthread

TARGET  := tancuri
SRC_DIR := src
OBJ_DIR := build

SRCS := $(wildcard $(SRC_DIR)/*.c)
OBJS := $(SRCS:$(SRC_DIR)/%.c=$(OBJ_DIR)/%.o)
DEPS := $(OBJS:.o=.d)

.PHONY: all run clean

all: $(TARGET)

$(TARGET): $(OBJS)
	$(CC) -o $@ $^ $(LDLIBS)

$(OBJ_DIR)/%.o: $(SRC_DIR)/%.c | $(OBJ_DIR)
	$(CC) $(CFLAGS) -MMD -MP -c $< -o $@

$(OBJ_DIR):
	mkdir -p $@

run: $(TARGET)
	./$(TARGET) A wsadf B ikjlh

clean:
	rm -rf $(OBJ_DIR) $(TARGET)

-include $(DEPS)
