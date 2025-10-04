CC ?= clang

# Build mode: default to release for maximum performance
BUILD ?= debug

ifeq ($(BUILD),debug)
CFLAGS = -std=c11 -Wall -Wextra -Wpedantic -g -O0 \
         -Wshadow -Wuninitialized -Wconversion -Wsign-conversion \
         -Wstrict-overflow=5 -Wcast-align -Wfloat-equal
LEAK_ENV = MallocStackLogging=1 ASAN_OPTIONS=detect_leaks=1
LDFLAGS =
else
# CFLAGS = -Wall -Wextra -O3 -march=native -flto -pipe
CFLAGS = -O3 -march=native -mtune=native -flto -pipe -fomit-frame-pointer \
         -funroll-loops -finline-functions
LEAK_ENV =
LDFLAGS =
endif

# Generate dependency files
CFLAGS += -MMD -MP

# Sources (update if you add/remove .c files)
SRCS = cpu.c cpuvm8.c
BUILD_DIR = build
OBJ_DIR = $(BUILD_DIR)/obj
BIN_DIR = $(BUILD_DIR)/bin

OBJS = $(patsubst %.c,$(OBJ_DIR)/%.o,$(SRCS))
DEPS = $(patsubst %.c,$(OBJ_DIR)/%.d,$(SRCS))

TARGET = $(BIN_DIR)/cpuvm8

.PHONY: all clean test

all: $(TARGET)

# Ensure directories exist
$(OBJ_DIR) $(BIN_DIR):
	mkdir -p $@

# Link step (put binary in build/bin)
$(TARGET): $(OBJS) | $(BIN_DIR)
	$(CC) $(LDFLAGS) -o $@ $^

# Compile step - object and dep files go under build/obj
$(OBJ_DIR)/%.o: %.c | $(OBJ_DIR)
	$(CC) $(CFLAGS) -c -o $@ $<

# Include auto-generated header dependency files (if present)
-include $(DEPS)

clean:
	rm -rf $(BUILD_DIR)

test: $(TARGET)
	$(LEAK_ENV) ./$(TARGET) cartridge.bin
