/*
 cpuVM8/microbench.c

 Minimal harness that accepts a user-supplied "flat" program array (sequence of
 bytes, typically 3-byte instructions: opcode, mode, operand) and builds two
 CPU templates:
   - normal template: program bytes copied verbatim into the code region
   - packed template: program interpreted as 3-byte instructions and converted
     into packed 2-byte form (packed(opcode,mode), operand) using the packing
     helper.

 The harness then runs both templates through the corresponding decoders:
   - cpu_step       : expects 3-byte instruction layout (opcode, mode, operand)
   - cpu_step_packed: expects packed 2-byte layout (packed(opcode,mode), operand)

 Usage:
   - Edit or replace the `program[]` array below with the bytes you want to
     benchmark. Example program layout (3-byte form):
       uint8_t program[] = {
         OPCODE_LDA, MODE_IMMEDIAT, 0x05,
         OPCODE_ADD, MODE_IMMEDIAT, 0x03,
         ...
       };
   - Build:
       clang -O3 -DNDEBUG -march=native -flto -o microbench microbench.c
   - Run:
       ./microbench                # run both decoders, default 10_000_000 steps
       ./microbench 5000000 42    # run both, 5M steps, seed 42
       ./microbench packed 2000000 123 debug  # run only packed, debug on
*/

#include <stdio.h>
#include <stdint.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include <math.h>
#include <mach/mach_time.h>

#include "../cpu.h"

/* ---------------- timing ---------------- */
static inline uint64_t now_ns(void) {
    static mach_timebase_info_data_t tb = {0,0};
    if (tb.denom == 0) mach_timebase_info(&tb);
    uint64_t t = mach_absolute_time();
    return (t * tb.numer) / tb.denom;
}

/* ---------------- constants derived from cpu.h ---------------- */
/* Code region size (0x00..0xEF) derived from STACK_BASE and STACK_SIZE in cpu.h */
#define CODE_SIZE ((uint8_t)(STACK_BASE - STACK_SIZE + 1)) /* should evaluate to 0xF0 */

/* ---------------- minimal debug control ---------------- */
static int dbg_enabled = 0;
static uint64_t dbg_interval = 1000000ULL;

/* Make final CPU state observable so compiler can't elide the work */
static volatile uint64_t final_pc_sink = 0;

/* Step function type */
typedef int (*step_fn)(CPU *);

/* Statistics tracking */
typedef struct {
    uint64_t min_ns;
    uint64_t max_ns;
    uint64_t median_ns;
    uint64_t mean_ns;
    double stddev_ns;
} stats_t;

/* ---------------- user program ----------------
   Replace the array below with the program you want to benchmark.
   It should be a sequence of bytes in the 3-byte instruction format:
     opcode, mode, operand, opcode, mode, operand, ...
*/
static uint8_t program[] = {
    /* Example: LDA #5 ; ADD #3 ; INX ; NOP */
    OPCODE_LDA, MODE_IMMEDIAT, 0x05,
    OPCODE_ADD, MODE_IMMEDIAT, 0x03,
    OPCODE_INX, MODE_IMMEDIAT, 0x00,
    OPCODE_NOP, MODE_IMMEDIAT, 0x00,
    /* you can extend / replace this with your own sequence */
};
static const size_t program_len = sizeof(program);

/* ---------------- helpers to build templates ---------------- */

/* Fill `tmpl` code region by repeating `program` sequence until CODE_SIZE is filled.
   This version treats the input program as a sequence of 3-byte instructions
   (opcode, mode, operand) and will never copy a partial instruction into the
   code region. If the program length is not a multiple of three, the trailing
   bytes are ignored. If there is not enough room to fit another full
   instruction near the end of the code region, the remaining bytes are filled
   with OPCODE_NOP values (so the code region contains no partial instruction).
*/
static void build_normal_template(CPU *tmpl, const uint8_t *prog, size_t len) {
    const size_t code_size = CODE_SIZE;
    size_t dst = 0;

    /* If there's no full instruction in the provided program, fill with NOPs */
    if (len < 3) {
        /* Fill entire code region with opcode NOP (single-byte 0x00).
           This also sets mode/operand bytes to NOP opcode value, which is
           fine: a NOP instruction will be interpreted as opcode=NOP and
           subsequent mode/operand bytes (also zero) behave as immediate/0. */
        memset(&tmpl->memory[0], (int)OPCODE_NOP, code_size);
        return;
    }

    /* Number of full 3-byte instructions available in the program */
    size_t triplets = len / 3;
    size_t triplet_idx = 0;

    /* Copy full triplets only; wrap triplet index if necessary. Stop when we
       cannot fit a full 3-byte instruction anymore. */
    while (dst + 2 < code_size) {
        size_t base = (triplet_idx % triplets) * 3;
        tmpl->memory[dst++] = prog[base + 0]; /* opcode */
        tmpl->memory[dst++] = prog[base + 1]; /* mode   */
        tmpl->memory[dst++] = prog[base + 2]; /* operand*/
        triplet_idx++;
    }

    /* If there are 1 or 2 bytes left at the end of code region, fill them with
       OPCODE_NOP so no partial instruction remains. */
    while (dst < code_size) {
        tmpl->memory[dst++] = (uint8_t)OPCODE_NOP;
    }

    /* Done: the code region now contains only full instructions (3-byte
       sequences) for the bulk, and the trailing bytes (if any) are NOPs. */
}

/* Build packed template from 3-byte program interpretation.
   For each triplet (opcode, mode, operand) we emit two bytes:
     dst: packed = PACK_INST_BYTE(opcode, mode)
     dst+1: operand
   If the program does not contain a full triplet at the end, we pad with NOPs.
   If opcode >= 32 or mode >= 8 or opcode == OPCODE_HALT, we emit a packed NOP. */
static void build_packed_template(CPU *tmpl, const uint8_t *prog, size_t len) {
    const size_t code_size = CODE_SIZE;
    size_t src = 0;
    size_t dst = 0;

    while (dst + 1 < code_size) {
        uint8_t opcode = OPCODE_NOP;
        uint8_t mode = MODE_IMMEDIAT;
        uint8_t operand = 0x00;

        if (src + 2 < len) {
            opcode = prog[src + 0];
            mode   = prog[src + 1];
            operand= prog[src + 2];
            src += 3;
        } else {
            /* not enough bytes for a triplet: emit NOP packed */
            opcode = OPCODE_NOP;
            mode = MODE_IMMEDIAT;
            operand = 0x00;
            /* advance src cyclically through program to keep predictable layout if desired */
            src = (src + 3) % (len ? len : 3);
        }

        if (opcode >= 32 || mode >= 8 || opcode == OPCODE_HALT) {
            /* fall back to packed NOP immediate */
            tmpl->memory[dst++] = PACK_INST_BYTE(OPCODE_NOP, MODE_IMMEDIAT);
            tmpl->memory[dst++] = 0x00;
        } else {
            tmpl->memory[dst++] = PACK_INST_BYTE(opcode, mode);
            tmpl->memory[dst++] = operand;
        }
    }

    /* If one byte leftover in code area, pad with a packed NOP (single-byte) */
    if (dst < code_size) {
        tmpl->memory[dst++] = PACK_INST_BYTE(OPCODE_NOP, MODE_IMMEDIAT);
    }
}

/* Optionally prefill stack (simple helper) */
static void prefill_stack(CPU *tmpl, unsigned seed, int items) {
    if (items <= 0) return;
    if (items > STACK_SIZE) items = STACK_SIZE;
    srand((unsigned)seed);
    for (int i = 0; i < items; ++i) {
        uint8_t addr = (uint8_t)(STACK_BASE - i);
        tmpl->memory[addr] = (uint8_t)(rand() & 0xFF);
    }
    tmpl->SP = (uint8_t)(STACK_BASE - items);
}

/* ---------------- validation helpers (moved out of main) ----------------
   These check templates for obvious encoding errors before running a long
   benchmark. This helps catch user mistakes in the program array.
*/
static int validate_normal_template(const CPU *tmpl) {
    int bad_count = 0;
    /* walk code region in 3-byte steps and verify opcode/mode ranges */
    for (size_t pc = 0; pc + 2 < (size_t)CODE_SIZE; pc += 3) {
        uint8_t opcode = tmpl->memory[pc];
        uint8_t mode = tmpl->memory[pc + 1];
        /* opcode must be within range and not be a hole caused by accidental data */
        if (opcode >= OPCODE_COUNT) {
            bad_count++;
            if (dbg_enabled) {
                printf("[validate] bad opcode 0x%02X at PC=0x%02zx\n", opcode, pc);
            }
            continue;
        }
        if (mode >= MODE_COUNT) {
            bad_count++;
            if (dbg_enabled) {
                printf("[validate] bad mode 0x%02X at PC=0x%02zx (opcode 0x%02X)\n", mode, pc, opcode);
            }
        }
    }
    return bad_count;
}

static int validate_packed_template(const CPU *tmpl) {
    int bad_count = 0;
    /* walk code region in 2-byte steps and verify unpacked opcode/mode */
    for (size_t pc = 0; pc + 1 < (size_t)CODE_SIZE; pc += 2) {
        uint8_t packed = tmpl->memory[pc];
        uint8_t opcode = (uint8_t)(packed & 0x1F);
        uint8_t mode = (uint8_t)((packed >> 5) & 0x07);
        if (opcode >= OPCODE_COUNT) {
            bad_count++;
            if (dbg_enabled) {
                printf("[validate] bad packed opcode 0x%02X at PC=0x%02zx\n", opcode, pc);
            }
            continue;
        }
        if (mode >= MODE_COUNT) {
            bad_count++;
            if (dbg_enabled) {
                printf("[validate] bad packed mode 0x%02X at PC=0x%02zx (opcode 0x%02X)\n", mode, pc, opcode);
            }
        }
    }
    return bad_count;
}

/* ---------------- optimized runner with minimal noise ----------------
   run_with_step keeps PC wrapped into the code region (CODE_SIZE) before each step
   so a microbenchmark cycles within the code area the user filled.
   Optimized for minimal noise in the hot loop.
*/
static uint64_t run_with_step(const CPU *template_cpu, step_fn step,
                              uint64_t max_steps, int reps,
                              uint64_t *out_steps, uint64_t *out_errors, uint64_t *out_halts) {
    uint64_t total_steps = 0;
    uint64_t total_errors = 0;
    uint64_t total_halts = 0;

    const uint8_t wrap_limit = (uint8_t)CODE_SIZE; /* wrap to CODE_SIZE */

    /* Pre-calculate debug parameters to minimize hot loop overhead */
    uint64_t next_debug_step = dbg_enabled ? dbg_interval : UINT64_MAX;

    uint64_t t0 = now_ns();
    for (int r = 0; r < reps; ++r) {
        CPU cpu;
        memcpy(&cpu, template_cpu, sizeof(CPU));
        cpu.PC = 0;
        for (uint64_t i = 0; i < max_steps; ++i) {
            /* Optimized PC wrapping: conditional is faster than modulo for most cases */
            if (cpu.PC >= wrap_limit) cpu.PC = 0;

            int res = step(&cpu);
            total_steps++;

            /* Optimized debug check: avoid modulo operation in hot loop */
            if (total_steps == next_debug_step) {
                printf("[dbg] steps=%llu PC=0x%02X A=0x%02X X=0x%02X SP=0x%02X\n",
                       (unsigned long long)total_steps, cpu.PC, cpu.A, cpu.X, cpu.SP);
                next_debug_step += dbg_interval;
            }

            if (res == CPU_HALTED) {
                total_errors++;
                if (dbg_enabled) {
                    uint8_t pc_show = cpu.PC;
                    printf("[dbg] CPU_HALTED at PC=0x%02X\n", pc_show);
                    printf("[dbg] mem@PC: %02X %02X %02X\n",
                           cpu.memory[pc_show],
                           cpu.memory[(uint8_t)(pc_show + 1)],
                           cpu.memory[(uint8_t)(pc_show + 2)]);
                    printf("[dbg] REGS A=0x%02X X=0x%02X PC=0x%02X SP=0x%02X FLAGS=0x%02X\n",
                           cpu.A, cpu.X, cpu.PC, cpu.SP, cpu.flags);
                }
                break;
            }
        }
        final_pc_sink += cpu.PC;
    }
    uint64_t t1 = now_ns();

    if (out_steps)  *out_steps  = total_steps;
    if (out_errors) *out_errors = total_errors;
    if (out_halts)  *out_halts  = total_halts;
    return t1 - t0;
}

/* ---------------- statistics helpers ---------------- */
static int compare_uint64(const void *a, const void *b) {
    uint64_t ua = *(const uint64_t*)a;
    uint64_t ub = *(const uint64_t*)b;
    if (ua < ub) return -1;
    if (ua > ub) return 1;
    return 0;
}

static void calculate_stats(uint64_t *times, int count, uint64_t total_steps, stats_t *stats) {
    if (count == 0) {
        memset(stats, 0, sizeof(*stats));
        return;
    }

    /* Sort times for median calculation */
    qsort(times, count, sizeof(uint64_t), compare_uint64);

    stats->min_ns = times[0];
    stats->max_ns = times[count - 1];
    stats->median_ns = times[count / 2];

    /* Calculate mean */
    uint64_t sum = 0;
    for (int i = 0; i < count; i++) {
        sum += times[i];
    }
    stats->mean_ns = sum / count;

    /* Calculate standard deviation */
    double variance = 0.0;
    for (int i = 0; i < count; i++) {
        double diff = (double)times[i] - (double)stats->mean_ns;
        variance += diff * diff;
    }
    stats->stddev_ns = sqrt(variance / count);
}

static void print_stats(const char *name, const stats_t *stats, uint64_t total_steps) {
    double min_ns_per = total_steps ? (double)stats->min_ns / (double)total_steps : 0.0;
    double median_ns_per = total_steps ? (double)stats->median_ns / (double)total_steps : 0.0;
    double mean_ns_per = total_steps ? (double)stats->mean_ns / (double)total_steps : 0.0;
    double max_ns_per = total_steps ? (double)stats->max_ns / (double)total_steps : 0.0;
    double cv = stats->mean_ns > 0 ? (stats->stddev_ns / stats->mean_ns) * 100.0 : 0.0;

    printf("%s:\n", name);
    printf("  min:    %12llu ns total (%8.6f ns/op)\n",
           (unsigned long long)stats->min_ns, min_ns_per);
    printf("  median: %12llu ns total (%8.6f ns/op)\n",
           (unsigned long long)stats->median_ns, median_ns_per);
    printf("  mean:   %12llu ns total (%8.6f ns/op) ±%.3f ns (CV: %.2f%%)\n",
           (unsigned long long)stats->mean_ns, mean_ns_per, stats->stddev_ns, cv);
    printf("  max:    %12llu ns total (%8.6f ns/op)\n",
           (unsigned long long)stats->max_ns, max_ns_per);
    printf("  steps:  %12llu\n", (unsigned long long)total_steps);
}

/* ---------------- main ---------------- */
int main(int argc, char **argv) {
    uint64_t cycles = 10000000ULL;
    unsigned seed = (unsigned)time(NULL);
    int run_packed_only = 0;
    int prefill = 0;
    int num_reps = 5; /* Multiple repetitions for statistical reliability */
    int diagnostic_mode = 0;

    /* simple arg parsing:
       ./microbench [packed] [cycles] [seed] [debug] [prefill] [reps=N]
    */
    for (int i = 1; i < argc; ++i) {
        if (strcmp(argv[i], "packed") == 0) { run_packed_only = 1; continue; }
        if (strcmp(argv[i], "debug") == 0) { dbg_enabled = 1; continue; }
        if (strcmp(argv[i], "prefill") == 0) { prefill = 1; continue; }
        if (strcmp(argv[i], "diag") == 0) { diagnostic_mode = 1; continue; }
        if (strncmp(argv[i], "reps=", 5) == 0) {
            int r = atoi(argv[i] + 5);
            if (r > 0) num_reps = r;
            continue;
        }
        /* numeric */
        char *end = NULL;
        long v = strtol(argv[i], &end, 10);
        if (end != argv[i]) {
            if (v > 0 && cycles == 10000000ULL) cycles = (uint64_t)v;
            else if (v > 0) seed = (unsigned)v;
        }
    }

    printf("microbench: seed=%u cycles=%llu mode=%s reps=%d\n", seed, (unsigned long long)cycles,
           run_packed_only ? "packed" : "both", num_reps);
    if (dbg_enabled) printf("microbench: debug enabled\n");
    if (diagnostic_mode) printf("microbench: diagnostic mode enabled\n");

    /* Build normal and packed templates from the single `program[]` array */
    CPU tmpl_normal;
    CPU tmpl_packed;
    initCPU(&tmpl_normal);
    initCPU(&tmpl_packed);

    build_normal_template(&tmpl_normal, program, program_len);
    build_packed_template(&tmpl_packed, program, program_len);

    if (prefill) {
        prefill_stack(&tmpl_normal, seed + 1, STACK_SIZE); /* full stack */
        prefill_stack(&tmpl_packed, seed + 2, STACK_SIZE);
    }

    int bad_normal = validate_normal_template(&tmpl_normal);
    int bad_packed = validate_packed_template(&tmpl_packed);
    if (bad_normal || bad_packed) {
        fprintf(stderr, "microbench: template validation failed: normal_bad=%d packed_bad=%d\n", bad_normal, bad_packed);
        if (!dbg_enabled) {
            fprintf(stderr, "microbench: run with 'debug' to see per-instruction diagnostics\n");
        }
        /* Don't abort automatically; user may want to inspect output.
           However, for safety we print a short summary. */
    } else if (dbg_enabled) {
        printf("[validate] templates ok: normal_bad=%d packed_bad=%d\n", bad_normal, bad_packed);
    }

    /* Enhanced warm-up to stabilize caches/BTB/branch predictors */
    printf("microbench: warming up caches and branch predictors...\n");
    uint64_t dummy_steps = 0, dummy_errors = 0, dummy_halts = 0;

    /* More aggressive warm-up for 3-byte decoder */
    if (!run_packed_only) {
        printf("microbench: warming up 3-byte decoder (more aggressive)...\n");
        (void)run_with_step(&tmpl_normal, cpu_step, 100000, 3, &dummy_steps, &dummy_errors, &dummy_halts);
    }

    /* Warm-up packed decoder */
    printf("microbench: warming up packed decoder...\n");
    (void)run_with_step(&tmpl_packed, cpu_step_packed, 100000, 2, &dummy_steps, &dummy_errors, &dummy_halts);

    printf("microbench: warm-up complete, starting measurements...\n");

    /* Normal 3-byte decoder with statistical analysis and burn-in */
    if (!run_packed_only) {
        uint64_t *times_normal = malloc((num_reps + 1) * sizeof(uint64_t));
        uint64_t total_steps_normal = 0;

        /* Burn-in run (not counted in statistics) */
        printf("microbench: 3-byte burn-in run...\n");
        uint64_t burnin_steps = 0, burnin_errors = 0, burnin_halts = 0;
        uint64_t burnin_time = run_with_step(&tmpl_normal, cpu_step, cycles, 1, &burnin_steps, &burnin_errors, &burnin_halts);
        if (diagnostic_mode) {
            printf("  3-byte burn-in: %llu ns (%.3f ns/op) [not counted]\n",
                   (unsigned long long)burnin_time,
                   (double)burnin_time / (double)burnin_steps);
        }

        /* Actual measurement runs */
        for (int rep = 0; rep < num_reps; rep++) {
            uint64_t steps3 = 0, errors3 = 0, halts3 = 0;
            times_normal[rep] = run_with_step(&tmpl_normal, cpu_step, cycles, 1, &steps3, &errors3, &halts3);
            if (rep == 0) total_steps_normal = steps3; /* Assume same for all reps */

            if (diagnostic_mode) {
                printf("  3-byte rep %d: %llu ns (%.3f ns/op)\n", rep + 1,
                       (unsigned long long)times_normal[rep],
                       (double)times_normal[rep] / (double)steps3);
            }

            if (errors3 > 0 || halts3 > 0) {
                printf("  rep %d: errors=%llu halts=%llu\n", rep + 1,
                       (unsigned long long)errors3, (unsigned long long)halts3);
            }
        }

        stats_t stats_normal;
        calculate_stats(times_normal, num_reps, total_steps_normal, &stats_normal);
        print_stats("cpu_step (3-byte)", &stats_normal, total_steps_normal);
        free(times_normal);
    }

    /* Packed decoder with statistical analysis and burn-in */
    {
        uint64_t *times_packed = malloc((num_reps + 1) * sizeof(uint64_t));
        uint64_t total_steps_packed = 0;

        /* Burn-in run (not counted in statistics) */
        printf("microbench: packed burn-in run...\n");
        uint64_t burnin_steps_p = 0, burnin_errors_p = 0, burnin_halts_p = 0;
        uint64_t burnin_time_p = run_with_step(&tmpl_packed, cpu_step_packed, cycles, 1, &burnin_steps_p, &burnin_errors_p, &burnin_halts_p);
        if (diagnostic_mode) {
            printf("  packed burn-in: %llu ns (%.3f ns/op) [not counted]\n",
                   (unsigned long long)burnin_time_p,
                   (double)burnin_time_p / (double)burnin_steps_p);
        }

        /* Actual measurement runs */
        for (int rep = 0; rep < num_reps; rep++) {
            uint64_t steps_p = 0, errors_p = 0, halts_p = 0;
            times_packed[rep] = run_with_step(&tmpl_packed, cpu_step_packed, cycles, 1, &steps_p, &errors_p, &halts_p);
            if (rep == 0) total_steps_packed = steps_p; /* Assume same for all reps */

            if (diagnostic_mode) {
                printf("  packed rep %d: %llu ns (%.3f ns/op)\n", rep + 1,
                       (unsigned long long)times_packed[rep],
                       (double)times_packed[rep] / (double)steps_p);
            }

            if (errors_p > 0 || halts_p > 0) {
                printf("  rep %d: errors=%llu halts=%llu\n", rep + 1,
                       (unsigned long long)errors_p, (unsigned long long)halts_p);
            }
        }

        stats_t stats_packed;
        calculate_stats(times_packed, num_reps, total_steps_packed, &stats_packed);
        print_stats("cpu_step_packed (2-byte)", &stats_packed, total_steps_packed);
        free(times_packed);
    }

    /* done */
    (void)final_pc_sink;
    return 0;
}
