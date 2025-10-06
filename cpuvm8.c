#include "cpuvm8.h"
#include "cpu.h"

int main(int argc, char *argv[]) {

  // Fréquence cible en MHz (modifiable dynamiquement)
  double freq_mhz = 4.0;
  int status = 0;
  int benchmark = 0;

  if (argc > 1) {
    freq_mhz = atof(argv[1]);
    if (freq_mhz < 0.01)
      freq_mhz = 0.01;
    benchmark = 1;
  }

  CPU cpu;
  initCPU(&cpu);

  // Programme de test (boucle simple)
  uint8_t program[] = {
      OPCODE_NOP,
      0,
      0 // End
  };

  memcpy(cpu.memory, program, sizeof(program));

  struct timespec start, now;
  clock_gettime(CLOCK_MONOTONIC, &start);

  for (int i = 0; i < INSTR_COUNT; i++) {

    status = cpu_step(&cpu);
    if (status == CPU_HALTED) {
      printf("CPU ERROR at PC=0x%02X\n", cpu.PC - 3);
      dump_cpu(&cpu);
      break;
    }

    // Calcul du temps cible pour ce cycle
    if (benchmark) {
      double target_time = (double)(i + 1) / (freq_mhz * 1e6);
      clock_gettime(CLOCK_MONOTONIC, &now);
      double elapsed =
          (now.tv_sec - start.tv_sec) + (now.tv_nsec - start.tv_nsec) / 1e9;

      // Si on va trop vite, on attend le temps manquant
      if (elapsed < target_time) {
        double sleep_time = target_time - elapsed;
        if (sleep_time > 0.00001) { // évite usleep trop courts
          usleep((useconds_t)(sleep_time * 1e6));
        }
      }
    }
  }

  clock_gettime(CLOCK_MONOTONIC, &now);
  double total_elapsed =
      (now.tv_sec - start.tv_sec) + (now.tv_nsec - start.tv_nsec) / 1e9;
  double mips = (double)INSTR_COUNT / (total_elapsed * 1e6);

  if (status != CPU_HALTED) {
    if (benchmark) {
      printf("Benchmark: %d instructions...\n", INSTR_COUNT);
      printf("Simulating CPU at %.2f MHz\n", freq_mhz);
    }
    printf("--------------------------------------------------\n");
    printf("Executed %d instructions in %.5f seconds\n", INSTR_COUNT,
           total_elapsed);
    printf("Estimated performance: %.2f MIPS (Millions of Instructions Per Second)\n",
           mips);
    printf("--------------------------------------------------\n");
  }
}

void dump_cpu(const CPU *cpu) {
  printf("=== CPU DUMP ===\n");
  printf("A:  0x%02X\n", cpu->A);
  printf("X:  0x%02X\n", cpu->X);
  printf("PC: 0x%02X\n", cpu->PC);
  printf("SP: 0x%02X\n", cpu->SP);
  printf("Flags: 0x%02X", cpu->flags);
  if (cpu->flags & FLAG_CARRY)
    printf(" CARRY");
  if (cpu->flags & FLAG_ZERO)
    printf(" ZERO");
  if (cpu->flags & FLAG_NEGATIVE)
    printf(" NEG");
  if (cpu->flags & FLAG_OVERFLOW)
    printf(" OVF");
  if (cpu->flags & FLAG_HALTED)
    printf(" ERROR");
  printf("\n");

  printf("Memory dump (hex):\n");
  for (int i = 0; i < MAX_MEMORY_SIZE; i += 16) {
    printf("%02X: ", i);
    for (int j = 0; j < 16; ++j) {
      printf("%02X ", cpu->memory[i + j]);
    }
    printf("\n");
  }
  printf("================\n");
}
