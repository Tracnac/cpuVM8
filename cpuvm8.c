#include "cpuvm8.h"

// Fréquence cible en MHz (modifiable dynamiquement)
double freq_mhz = 16.0;

int main(int argc, char *argv[]) {
    (void)argc; (void)argv;

    if (argc > 1) {
        freq_mhz = atof(argv[1]);
        if (freq_mhz < 0.01) freq_mhz = 0.01;
    }

    CPU cpu;
    initCPU(&cpu);

    // Programme de test (boucle simple)
    cpu.memory[0] = OPCODE_LDA;
    cpu.memory[1] = MODE_IMM;
    cpu.memory[2] = 42;
    cpu.memory[3] = OPCODE_B;
    cpu.memory[4] = COND_AL;
    cpu.memory[5] = 0;  // Jump à 0 (boucle infinie)

    printf("Simulating CPU at %.2f MHz\n", freq_mhz);
    printf("Benchmark: %d instructions...\n", INSTR_COUNT);

    struct timespec start, now;
    clock_gettime(CLOCK_MONOTONIC, &start);

    for (int i = 0; i < INSTR_COUNT; i++) {
        cpu_step(&cpu);

        // Calcul du temps cible pour ce cycle
        double target_time = (double)(i + 1) / (freq_mhz * 1e6);
        clock_gettime(CLOCK_MONOTONIC, &now);
        double elapsed = (now.tv_sec - start.tv_sec) + (now.tv_nsec - start.tv_nsec) / 1e9;

        // Si on va trop vite, on attend le temps manquant
        if (elapsed < target_time) {
            double sleep_time = target_time - elapsed;
            if (sleep_time > 0.00001) { // évite usleep trop courts
                usleep((useconds_t)(sleep_time * 1e6));
            }
        }
        // Si on est en retard, on ne fait rien (on rattrape au cycle suivant)
    }

    clock_gettime(CLOCK_MONOTONIC, &now);
    double total_elapsed = (now.tv_sec - start.tv_sec) + (now.tv_nsec - start.tv_nsec) / 1e9;
    double mips = (double)INSTR_COUNT / (total_elapsed * 1e6);

    printf("--------------------------------------------------\n");
    printf("Executed %d instructions in %.5f seconds\n", INSTR_COUNT, total_elapsed);
    printf("Simulated frequency: %.2f MHz\n", freq_mhz);
    printf("Performance: %.2f MIPS (Millions of Instructions Per Second)\n", mips);
    printf("--------------------------------------------------\n");
}
