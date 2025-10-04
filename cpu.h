#ifndef CPU_H
#define CPU_H

#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>

// Plus simple tu meurs
/*
 * 0x00-0x7F : Code (128 bytes)
 * 0x80-0xEF : Data/Heap (112 bytes)
 * 0xF0-0xFF : Stack (16 bytes)
 */

// Retour de cpu_step
enum { CPU_ERROR = -1, CPU_OK = 0, CPU_HALT = 1 };

// Memory layout
enum { CODE_BASE = 0x0, STACK_BASE = 0xFF, STACK_SIZE=2, MAX_MEMORY_SIZE = 256 };

// Registers
enum {
    REG_A = 0,
    REG_X,
    REG_PC, // Program Counter
    REG_SP, // Stack Pointer
};

// Flags
enum {
    FLAG_CARRY     = 1 << 0,
    FLAG_ZERO      = 1 << 1,
    FLAG_NEGATIVE  = 1 << 2,
    FLAG_OVERFLOW  = 1 << 3
};

// Opcodes
enum {
    OPCODE_NOP     = 0x00, // No Operation
    OPCODE_LDA     = 0x01, // Load Accumulator
    OPCODE_LDX     = 0x02, // Load Register X
    OPCODE_STA     = 0x03, // Store Accumulator
    OPCODE_STX     = 0x04, // Store Register X
    OPCODE_ADD     = 0x05, // Add
    OPCODE_SUB     = 0x06, // Subtract
    OPCODE_XOR     = 0x07, // Exclusive OR
    OPCODE_AND     = 0x08, // AND
    OPCODE_OR      = 0x09, // OR
    OPCODE_B       = 0x0A, // B NE, B EQ, B AL ...
    OPCODE_POP     = 0x0B, // Pop from Stack
    OPCODE_PUSH    = 0x0C, // Push to Stack
    OPCODE_HALT    = 0x0F  // Halt
};

// Addressing modes
enum {
    MODE_IMM = 0x00,  // Immediate:  #value
    MODE_ABS = 0x01,  // Absolute:   address
    MODE_IDX = 0x02,  // Indexed:    address,X
    MODE_IND = 0x03,  // Indirect:   [$address]
};

// Branch conditions (byte après OPCODE_B)
enum {
    COND_AL  = 0x00,  // Always (unconditional jump)
    COND_EQ  = 0x01,  // Equal (Z=1)
    COND_NE  = 0x02,  // Not Equal (Z=0)
    COND_CS  = 0x03,  // Carry Set (C=1)
    COND_CC  = 0x04,  // Carry Clear (C=0)
    COND_MI  = 0x05,  // Minus/Negative (N=1)
    COND_PL  = 0x06,  // Plus/Positive (N=0)
};

// CPU
typedef struct {
    uint8_t A;      // Accumulator
    uint8_t X;      // Index register
    uint8_t PC;     // Program Counter
    uint8_t SP;     // Stack Pointer
    uint8_t flags;  // Status flags
    uint8_t memory[MAX_MEMORY_SIZE];
} CPU __attribute__((aligned(64)));

// Définir les handlers pour chaque opcode
typedef void (*opcode_handler)(CPU*, uint8_t, uint8_t);

// ============================================================================
// OPCODE HANDLERS
// ============================================================================

static void op_nop(CPU *cpu, uint8_t mode, uint8_t operand) {
    (void)cpu; (void)mode; (void)operand;
}

static inline void op_lda(CPU *cpu, uint8_t mode, uint8_t operand) {
    if (mode == MODE_IMM) {
        cpu->A = operand;
    } else if (mode == MODE_ABS) {
        cpu->A = cpu->memory[operand];
    } else if (mode == MODE_IDX) {
        cpu->A = cpu->memory[(operand + cpu->X) & 0xFF];
    } else if (mode == MODE_IND) {
        cpu->A = cpu->memory[cpu->memory[operand]];
    }

    // Update flags Zero et Sign
    cpu->flags &= ~(FLAG_ZERO | FLAG_NEGATIVE);
    if (cpu->A == 0)
        cpu->flags |= FLAG_ZERO;
    if (cpu->A & 0x80)
        cpu->flags |= FLAG_NEGATIVE;
}

static void op_ldx(CPU *cpu, uint8_t mode, uint8_t operand) {
    if (mode == MODE_IMM) {
        cpu->X = operand;
    } else if (mode == MODE_ABS) {
        cpu->X = cpu->memory[operand];
    } else if (mode == MODE_IDX) {
        cpu->X = cpu->memory[(operand + cpu->X) & 0xFF];
    } else if (mode == MODE_IND) {
        cpu->X = cpu->memory[cpu->memory[operand]];
    }

    cpu->flags &= ~(FLAG_ZERO | FLAG_NEGATIVE);
    if (cpu->X == 0)
        cpu->flags |= FLAG_ZERO;
    if (cpu->X & 0x80)
        cpu->flags |= FLAG_NEGATIVE;
}

static void op_sta(CPU *cpu, uint8_t mode, uint8_t operand) {
    if (mode == MODE_ABS) {
        cpu->memory[operand] = cpu->A;
    } else if (mode == MODE_IDX) {
        cpu->memory[(operand + cpu->X) & 0xFF] = cpu->A;
    } else if (mode == MODE_IND) {
        cpu->memory[cpu->memory[operand]] = cpu->A;
    }
    // STA ne modifie pas les flags
}

static void op_stx(CPU *cpu, uint8_t mode, uint8_t operand) {
    if (mode == MODE_ABS) {
        cpu->memory[operand] = cpu->X;
    } else if (mode == MODE_IDX) {
        cpu->memory[(operand + cpu->X) & 0xFF] = cpu->X;
    } else if (mode == MODE_IND) {
        cpu->memory[cpu->memory[operand]] = cpu->X;
    }
    // STX ne modifie pas les flags
}

static void op_add(CPU *cpu, uint8_t mode, uint8_t operand) {
    uint8_t value;
    if (mode == MODE_IMM) {
        value = operand;
    } else if (mode == MODE_ABS) {
        value = cpu->memory[operand];
    } else if (mode == MODE_IDX) {
        value = cpu->memory[(operand + cpu->X) & 0xFF];
    } else if (mode == MODE_IND) {
        value = cpu->memory[cpu->memory[operand]];
    }

    uint16_t result = cpu->A + value;
    cpu->A = result & 0xFF;

    // Update flags
    cpu->flags &= ~(FLAG_ZERO | FLAG_NEGATIVE | FLAG_CARRY | FLAG_OVERFLOW);

    if (cpu->A == 0)
        cpu->flags |= FLAG_ZERO;
    if (cpu->A & 0x80)
        cpu->flags |= FLAG_NEGATIVE;
    if (result > 0xFF)
        cpu->flags |= FLAG_CARRY;

    // Overflow: signes identiques donnent résultat de signe différent
    if (((cpu->A ^ value) & 0x80) == 0 && ((cpu->A ^ result) & 0x80) != 0)
        cpu->flags |= FLAG_OVERFLOW;
}

static void op_sub(CPU *cpu, uint8_t mode, uint8_t operand) {
    uint8_t value;
    if (mode == MODE_IMM) {
        value = operand;
    } else if (mode == MODE_ABS) {
        value = cpu->memory[operand];
    } else if (mode == MODE_IDX) {
        value = cpu->memory[(operand + cpu->X) & 0xFF];
    } else if (mode == MODE_IND) {
        value = cpu->memory[cpu->memory[operand]];
    }

    uint16_t result = cpu->A - value;
    cpu->A = result & 0xFF;

    // Update flags
    cpu->flags &= ~(FLAG_ZERO | FLAG_NEGATIVE | FLAG_CARRY | FLAG_OVERFLOW);

    if (cpu->A == 0)
        cpu->flags |= FLAG_ZERO;
    if (cpu->A & 0x80)
        cpu->flags |= FLAG_NEGATIVE;
    if (result < 0x100)  // Pas de borrow
        cpu->flags |= FLAG_CARRY;

    // Overflow pour soustraction
    if (((cpu->A ^ value) & 0x80) != 0 && ((cpu->A ^ result) & 0x80) != 0)
        cpu->flags |= FLAG_OVERFLOW;
}

static void op_xor(CPU *cpu, uint8_t mode, uint8_t operand) {
    uint8_t value;
    if (mode == MODE_IMM) {
        value = operand;
    } else if (mode == MODE_ABS) {
        value = cpu->memory[operand];
    } else if (mode == MODE_IDX) {
        value = cpu->memory[(operand + cpu->X) & 0xFF];
    } else if (mode == MODE_IND) {
        value = cpu->memory[cpu->memory[operand]];
    }

    cpu->A ^= value;

    cpu->flags &= ~(FLAG_ZERO | FLAG_NEGATIVE);
    if (cpu->A == 0)
        cpu->flags |= FLAG_ZERO;
    if (cpu->A & 0x80)
        cpu->flags |= FLAG_NEGATIVE;
}

static void op_and(CPU *cpu, uint8_t mode, uint8_t operand) {
    uint8_t value;
    if (mode == MODE_IMM) {
        value = operand;
    } else if (mode == MODE_ABS) {
        value = cpu->memory[operand];
    } else if (mode == MODE_IDX) {
        value = cpu->memory[(operand + cpu->X) & 0xFF];
    } else if (mode == MODE_IND) {
        value = cpu->memory[cpu->memory[operand]];
    }

    cpu->A &= value;

    cpu->flags &= ~(FLAG_ZERO | FLAG_NEGATIVE);
    if (cpu->A == 0)
        cpu->flags |= FLAG_ZERO;
    if (cpu->A & 0x80)
        cpu->flags |= FLAG_NEGATIVE;
}

static void op_or(CPU *cpu, uint8_t mode, uint8_t operand) {
    uint8_t value;
    if (mode == MODE_IMM) {
        value = operand;
    } else if (mode == MODE_ABS) {
        value = cpu->memory[operand];
    } else if (mode == MODE_IDX) {
        value = cpu->memory[(operand + cpu->X) & 0xFF];
    } else if (mode == MODE_IND) {
        value = cpu->memory[cpu->memory[operand]];
    }

    cpu->A |= value;

    cpu->flags &= ~(FLAG_ZERO | FLAG_NEGATIVE);
    if (cpu->A == 0)
        cpu->flags |= FLAG_ZERO;
    if (cpu->A & 0x80)
        cpu->flags |= FLAG_NEGATIVE;
}

static void op_branch(CPU *cpu, uint8_t condition, uint8_t address) {
    int should_branch = 0;

    switch(condition) {
        case COND_AL:  // Always
            should_branch = 1;
            break;
        case COND_EQ:  // Equal (Z=1)
            should_branch = (cpu->flags & FLAG_ZERO) != 0;
            break;
        case COND_NE:  // Not Equal (Z=0)
            should_branch = (cpu->flags & FLAG_ZERO) == 0;
            break;
        case COND_CS:  // Carry Set (C=1)
            should_branch = (cpu->flags & FLAG_CARRY) != 0;
            break;
        case COND_CC:  // Carry Clear (C=0)
            should_branch = (cpu->flags & FLAG_CARRY) == 0;
            break;
        case COND_MI:  // Minus/Negative (N=1)
            should_branch = (cpu->flags & FLAG_NEGATIVE) != 0;
            break;
        case COND_PL:  // Plus/Positive (N=0)
            should_branch = (cpu->flags & FLAG_NEGATIVE) == 0;
            break;
    }

    if (should_branch) {
        cpu->PC = address;
    }
}

static void op_push(CPU *cpu, uint8_t mode, uint8_t operand) {
    (void)mode; (void)operand;  // PUSH pousse toujours A

    if (cpu->SP < (STACK_BASE - STACK_SIZE)) {  // Stack overflow check
        return;
    }

    cpu->memory[cpu->SP] = cpu->A;
    cpu->SP--;
}

static void op_pop(CPU *cpu, uint8_t mode, uint8_t operand) {
    (void)mode; (void)operand;  // POP dépile toujours vers A

    if (cpu->SP >= STACK_BASE) {  // Stack underflow check
        return;
    }

    cpu->SP++;
    cpu->A = cpu->memory[cpu->SP];

    // Update flags
    cpu->flags &= ~(FLAG_ZERO | FLAG_NEGATIVE);
    if (cpu->A == 0)
        cpu->flags |= FLAG_ZERO;
    if (cpu->A & 0x80)
        cpu->flags |= FLAG_NEGATIVE;
}

static void op_halt(CPU *cpu, uint8_t mode, uint8_t operand) {
    (void)cpu; (void)mode; (void)operand;
    // Le flag HALT sera géré dans cpu_step
}

// Table de dispatch
static const opcode_handler handlers[16] = {
    [OPCODE_NOP]  = op_nop,
    [OPCODE_LDA]  = op_lda,
    [OPCODE_LDX]  = op_ldx,
    [OPCODE_ADD]  = op_add,
    [OPCODE_SUB]  = op_sub,
    [OPCODE_XOR]  = op_xor,
    [OPCODE_STA]  = op_sta,
    [OPCODE_STX]  = op_stx,
    [OPCODE_AND]  = op_and,
    [OPCODE_OR]   = op_or,
    [OPCODE_B]    = op_branch,
    [OPCODE_POP]  = op_pop,
    [OPCODE_PUSH] = op_push,
    [OPCODE_HALT] = op_halt,
};

// CPU initialization
static inline void initCPU(CPU *cpu) {
    __builtin_memset(cpu, 0, sizeof(CPU));
    cpu->SP = STACK_BASE;
}

// cpu_step
static inline int cpu_step(CPU *cpu) {
    // if (!cpu) {
    //     return -1;
    // }

    // Fetch
    uint8_t opcode = cpu->memory[cpu->PC++];

    // Check HALT
    // if (opcode == OPCODE_HALT) {
    //     return CPU_HALT;  // Signal HALT
    // }

    // Fetch mode et operand
    uint8_t mode = cpu->memory[cpu->PC++];
    uint8_t operand = cpu->memory[cpu->PC++];

    // Execute
    if (opcode < 16 && handlers[opcode]) {
        handlers[opcode](cpu, mode, operand);
        return 0;  // Success
    }

    return -1;  // Invalid opcode
}

static inline void cpu_run(CPU *cpu) {
    // if (!cpu) {
    //     return;
    // }

    int result;
    while (1) {
        result = cpu_step(cpu);

        if (result == 1) {
            // HALT rencontré
            printf("CPU HALTED at PC=0x%02X\n", cpu->PC - 1);
            break;
        } else if (result == -1) {
            // Erreur
            printf("CPU ERROR at PC=0x%02X\n", cpu->PC - 3);
            break;
        }

        // Protection contre les boucles infinies
        // (optionnel, à retirer en production)
        // static int step_count = 0;
        // if (++step_count > 100000) {
        //     printf("CPU: Max steps reached, stopping\n");
        //     break;
        // }
    }
}

#endif // CPU_H
