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
CFLAGS = -O3 -march=native -mtune=native -flto -pipe -fomit-frame-pointer \
         -funroll-loops -finline-functions
LEAK_ENV =
LDFLAGS =
endif

# Generate dependency files
CFLAGS += -MMD -MP

# Sources (update if you add/remove .c files)
# Split sources into library (no main) and app (contains main) so tests
# can link only the library objects and avoid duplicate `main` symbols.
APP_SRCS = cpuvm8.c

LIB_SRCS = cpu.c
SRCS = $(LIB_SRCS) $(APP_SRCS)

BUILD_DIR = build
OBJ_DIR = $(BUILD_DIR)/obj
BIN_DIR = $(BUILD_DIR)/bin

OBJS = $(patsubst %.c,$(OBJ_DIR)/%.o,$(SRCS))
DEPS = $(patsubst %.c,$(OBJ_DIR)/%.d,$(SRCS))

TARGET = $(BIN_DIR)/cpuvm8

# --- TESTS AUTOMATION ---
# Find all test source files matching *_test.c
TEST_SRCS := $(wildcard tests/*_test.c)
TEST_OBJS := $(patsubst tests/%.c,$(OBJ_DIR)/%.o,$(TEST_SRCS))

UNITY_SRC = tests/unity/unity.c
UNITY_OBJ = $(OBJ_DIR)/unity.o

# Add main test runner object (assumed to be cpuVM8_test.c)

TEST_RUNNER_OBJ = $(OBJ_DIR)/cpuVM8_test.o

# Add source objects needed for tests: only library sources (no main)
SRC_OBJS = $(patsubst %.c,$(OBJ_DIR)/%.o,$(LIB_SRCS))

TEST_BIN = $(BIN_DIR)/cpuVM8_test

# Include auto-generated header dependency files (if present)
-include $(DEPS)

.PHONY: all clean run tests

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

# Compile test sources
$(OBJ_DIR)/%.o: tests/%.c | $(OBJ_DIR)
	$(CC) $(CFLAGS) -c -o $@ $<

# Compile Unity
$(OBJ_DIR)/unity.o: $(UNITY_SRC) | $(OBJ_DIR)
	$(CC) $(CFLAGS) -c -o $@ $<

# Link all test objects, Unity, and source objects into test binary
$(TEST_BIN): $(TEST_RUNNER_OBJ) $(TEST_OBJS) $(SRC_OBJS) $(UNITY_OBJ) | $(BIN_DIR)
	$(CC) $(LDFLAGS) -o $@ $^

clean:
	rm -rf $(BUILD_DIR)

run: $(TARGET)
	$(LEAK_ENV) ./$(TARGET) 8

tests: $(TEST_BIN)
	./$(TEST_BIN)
