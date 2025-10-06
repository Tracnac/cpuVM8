#include "../cpu.h"

// Test programs
static void load_simple_loop(CPU *cpu) {
  // Simple counting loop: count from 100 down to 0
  cpu->memory[0xF0] = 100; // Counter

  uint8_t program[] = {
      OPCODE_LDX,
      MODE_ABSOLUTE,
      0xF0, // Load counter into X
      OPCODE_DEX,
      0,
      0, // Decrement X
      OPCODE_STX,
      MODE_ABSOLUTE,
      0xF0, // Store X back to counter
      OPCODE_CPX,
      MODE_IMMEDIAT,
      0, // Compare X with 0
      OPCODE_B,
      COND_NE,
      3, // Jump back to DEX if not zero
      OPCODE_HALT,
      0,
      0 // End
  };

  memcpy(cpu->memory, program, sizeof(program));
}

static void load_fibonacci_program(CPU *cpu) {
  // Calculate fibonacci numbers
  cpu->memory[0xF0] = 0;  // F(n-2)
  cpu->memory[0xF1] = 1;  // F(n-1)
  cpu->memory[0xF2] = 15; // Counter

  uint8_t program[] = {
      OPCODE_LDA,
      MODE_ABSOLUTE,
      0xF0, // Load F(n-2)
      OPCODE_ADD,
      MODE_ABSOLUTE,
      0xF1, // Add F(n-1)
      OPCODE_STA,
      MODE_ABSOLUTE,
      0xF3, // Store F(n)

      OPCODE_LDA,
      MODE_ABSOLUTE,
      0xF1, // Load F(n-1)
      OPCODE_STA,
      MODE_ABSOLUTE,
      0xF0, // Store as new F(n-2)
      OPCODE_LDA,
      MODE_ABSOLUTE,
      0xF3, // Load F(n)
      OPCODE_STA,
      MODE_ABSOLUTE,
      0xF1, // Store as new F(n-1)

      OPCODE_LDX,
      MODE_ABSOLUTE,
      0xF2, // Load counter
      OPCODE_DEX,
      0,
      0, // Decrement X
      OPCODE_STX,
      MODE_ABSOLUTE,
      0xF2, // Store counter

      OPCODE_CPX,
      MODE_IMMEDIAT,
      0, // Compare with 0
      OPCODE_B,
      COND_NE,
      0, // Jump back to start if not zero

      OPCODE_HALT,
      0,
      0 // End
  };

  memcpy(cpu->memory, program, sizeof(program));
}

static void load_arithmetic_program(CPU *cpu) {
  // Intensive arithmetic operations
  cpu->memory[0xF0] = 50; // Counter
  cpu->memory[0xF1] = 7;  // Value to add
  cpu->memory[0xF2] = 3;  // Value to subtract

  uint8_t program[] = {
      OPCODE_LDA,
      MODE_IMMEDIAT,
      1, // Start with A = 1
      OPCODE_ADD,
      MODE_ABSOLUTE,
      0xF1, // Add 7
      OPCODE_SUB,
      MODE_ABSOLUTE,
      0xF2, // Subtract 3
      OPCODE_ADD,
      MODE_IMMEDIAT,
      2, // Add 2

      OPCODE_LDX,
      MODE_ABSOLUTE,
      0xF0, // Load counter
      OPCODE_DEX,
      0,
      0, // Decrement
      OPCODE_STX,
      MODE_ABSOLUTE,
      0xF0, // Store back
      OPCODE_CPX,
      MODE_IMMEDIAT,
      0, // Check if done
      OPCODE_B,
      COND_NE,
      3, // Jump back if not zero

      OPCODE_HALT,
      0,
      0 // End
  };

  memcpy(cpu->memory, program, sizeof(program));
}

static double benchmark_cpu(void (*init_func)(CPU *), int (*step_func)(CPU *),
                            void (*load_func)(CPU *), const char *test_name,
                            int iterations) {
  CPU cpu;
  clock_t start, end;
  long total_cycles = 0;

  printf("Running %s (%d iterations)...\n", test_name, iterations);

  start = clock();

  for (int i = 0; i < iterations; i++) {
    init_func(&cpu);
    load_func(&cpu);

    int cycles = 0;
    int result;
    while ((result = step_func(&cpu)) == 0) {
      cycles++;
    }
    total_cycles += cycles;
  }

  end = clock();
  double time_taken = ((double)(end - start)) / CLOCKS_PER_SEC;

  printf("  Time: %.6f seconds\n", time_taken);
  printf("  Total cycles: %ld\n", total_cycles);
  if (time_taken > 0) {
    printf("  Cycles per second: %.0f\n", total_cycles / time_taken);
    printf("  Estimated MIPS: %.2f\n", total_cycles / (time_taken * 1e6));
  }

  return time_taken;
}

int main(int argc, char *argv[]) {
  int iterations = 5000;
  double total_time = 0;

  if (argc > 1) {
    iterations = atoi(argv[1]);
    if (iterations <= 0)
      iterations = 5000;
  }

  // Benchmark function pointer
  static void (*benchmark[])(CPU *) = {
      load_simple_loop,
      load_fibonacci_program,
      load_arithmetic_program
  };

  printf("=== CPU Performance Benchmark ===\n");
  printf("Iterations per test: %d\n\n", iterations);

  size_t num_benchmarks = sizeof(benchmark) / sizeof(benchmark[0]);
  for (int i = 0; i < (int)num_benchmarks; ++i) {
  printf("TEST %d: \n", i);
  printf("----------------------------\n");
  double time = benchmark_cpu(initCPU, cpu_step, benchmark[i],
                                     "Normal CPU (switch)", iterations);
  total_time += time;
}

  // Overall results
  printf("\n=== SUMMARY ===\n");
  printf("Total normal time: %.6f seconds\n", total_time);

  // Build info
  printf("\n=== BUILD INFO ===\n");
#ifdef __GNUC__
  printf("Compiler: GCC %d.%d.%d\n", __GNUC__, __GNUC_MINOR__,
         __GNUC_PATCHLEVEL__);
#endif
#ifdef __clang__
  printf("Compiler: Clang %s\n", __clang_version__);
#endif
#ifdef __OPTIMIZE__
  printf("Optimization: Enabled\n");
#else
  printf("Optimization: Disabled\n");
#endif

  return 0;
}
