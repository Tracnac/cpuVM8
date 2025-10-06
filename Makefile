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

CFLAGS_OPT = -O3 -march=native -mtune=native -flto -pipe -fomit-frame-pointer \
         -funroll-loops -finline-functions

# Generate dependency files
CFLAGS += -MMD -MP

# Sources (update if you add/remove .c files)
# Split sources into library (no main) and app (contains main) so tests
# can link only the library objects and avoid duplicate `main` symbols.
APP_SRCS = cpuvm8.c

LIB_SRCS = cpu.c
BENCH_SRCS = benchmark.c
SRCS = $(LIB_SRCS) $(APP_SRCS)

BUILD_DIR = build/$(BUILD)
OBJ_DIR = $(BUILD_DIR)/obj
BIN_DIR = $(BUILD_DIR)/bin

OBJS = $(patsubst %.c,$(OBJ_DIR)/%.o,$(SRCS))
DEPS = $(patsubst %.c,$(OBJ_DIR)/%.d,$(SRCS))

TARGET = $(BIN_DIR)/cpuvm8
BENCH_TARGET = benchmark
MICROBENCH_TARGET = microbenchmark

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

.PHONY: all clean run tests benchmark help status debug release tests-debug tests-release run-debug run-release benchmark-debug benchmark-release

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
	rm -rf build

run: $(TARGET)
	$(LEAK_ENV) ./$(TARGET) 8

tests: $(TEST_BIN)
	./$(TEST_BIN)

# Benchmark targets is always built with optimizations
$(BENCH_TARGET): tools/benchmark.c | $(BIN_DIR)
	$(CC) $(CFLAGS_OPT) -o build/benchmark $<

# benchmark: $(BENCH_TARGET)
# 	./$(BENCH_TARGET)

# microbenchmark targets is always built with optimizations
$(MICROBENCH_TARGET): tools/microbenchmark.c | $(BIN_DIR)
		$(CC) $(CFLAGS_OPT) -o build/microbenchmark $<

# microbenchmark: $(MICROBENCH_TARGET)
# 		./$(BENCH_TARGET)

help:
	@echo ""
	@echo "Available targets:"
	@echo "  all          - Build main application (default)"
	@echo "  tests        - Build and run tests"
	@echo "  benchmark    - Build and run performance benchmark"
	@echo "  run          - Build and run main application"
	@echo "  clean        - Remove all build directories"
	@echo "  status       - Show current build configuration"
	@echo "  help         - Show this help message"
	@echo ""
	@echo "Easy build mode shortcuts:"
	@echo "  debug              - Build main app in debug mode"
	@echo "  release            - Build main app in release mode"
	@echo "  tests-debug        - Build and run tests in debug mode"
	@echo "  tests-release      - Build and run tests in release mode"
	@echo "  benchmark          - Build benchmark in release mode"
	@echo "  microbenchmark     - Build microbenchmark in release mode"
	@echo "  run-debug          - Build and run main app in debug mode"
	@echo "  run-release        - Build and run main app in release mode"
	@echo ""

status:
	@echo "=== BUILD STATUS ==="
	@echo ""
	@echo "DEBUG BUILD:"
	@if [ -f "build/debug/bin/cpuvm8" ]; then echo "  ✓ Main binary: build/debug/bin/cpuvm8"; else echo "  ✗ Main binary: not built"; fi
	@if [ -f "build/debug/bin/cpuVM8_test" ]; then echo "  ✓ Test binary: build/debug/bin/cpuVM8_test"; else echo "  ✗ Test binary: not built"; fi
	@echo ""
	@echo "RELEASE BUILD:"
	@if [ -f "build/release/bin/cpuvm8" ]; then echo "  ✓ Main binary: build/release/bin/cpuvm8"; else echo "  ✗ Main binary: not built"; fi
	@if [ -f "build/release/bin/cpuVM8_test" ]; then echo "  ✓ Test binary: build/release/bin/cpuVM8_test"; else echo "  ✗ Test binary: not built"; fi
	@echo ""
	@echo "Default build mode for 'make' commands: $(BUILD)"

# Convenient aliases for debug/release builds
debug:
	$(MAKE) BUILD=debug all

release:
	$(MAKE) BUILD=release all

tests-debug:
	$(MAKE) BUILD=debug tests

tests-release:
	$(MAKE) BUILD=release tests

run-debug:
	$(MAKE) BUILD=debug run

run-release:
	$(MAKE) BUILD=release run
