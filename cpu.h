#ifndef CPU_H
#define CPU_H

#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>

// Plus simple tu meurs
/*
 * 0x00-0xEF : Code (240 bytes)
 * 0xF0-0xFF : Stack (16 bytes)
 */

// Retour de cpu_step
enum { CPU_ERROR = -1, CPU_OK = 0, CPU_HALT = 1 };

// Memory layout
enum { CODE_BASE = 0x0, STACK_BASE = 0xFF, STACK_SIZE=16, MAX_MEMORY_SIZE = 256 };

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
    FLAG_OVERFLOW  = 1 << 3,
    FLAG_ERROR     = 1 << 4
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
    OPCODE_CMP     = 0x0D, // Compare (sets flags without storing result)
    OPCODE_CPX     = 0x0E, // Compare X register (sets flags without storing result)
    OPCODE_HALT    = 0x0F  // Halt
};

// Addressing modes
enum {
    MODE_IMMEDIAT  = 0x00,  // Immediate:  #value
    MODE_ABSOLUTE  = 0x01,  // Absolute:   address
    MODE_INDEXED_X  = 0x02,  // Indexed:    address,X
    MODE_INDIRECT  = 0x03,  // Indirect:   [$address]
    MODE_INDIRECT_INDEXED_X = 0x04,  // Indirect Indexed: [$address,X]
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
typedef int (*opcode_handler)(CPU*, uint8_t, uint8_t);

// ============================================================================
// OPCODE HANDLERS
// ============================================================================

static inline int op_nop(CPU *cpu, uint8_t mode, uint8_t operand) {
    (void)cpu; (void)mode; (void)operand;
    return CPU_OK;
}

static inline int op_lda(CPU *cpu, uint8_t mode, uint8_t operand) {
    switch (mode) {
        case MODE_IMMEDIAT:
            cpu->A = operand;
            break;
        case MODE_ABSOLUTE:
            cpu->A = cpu->memory[operand];
            break;
        case MODE_INDEXED_X:
            cpu->A = cpu->memory[(operand + cpu->X) & 0xFF];
            break;
        case MODE_INDIRECT:
            cpu->A = cpu->memory[cpu->memory[operand]];
            break;
        case MODE_INDIRECT_INDEXED_X:
            cpu->A = cpu->memory[cpu->memory[(operand + cpu->X) & 0xFF]];
            break;
        default:
            cpu->flags |= FLAG_ERROR;
            return CPU_ERROR;
    }

    // Update flags Zero et Sign
    cpu->flags &= ~(FLAG_ZERO | FLAG_NEGATIVE);
    if (cpu->A == 0)
        cpu->flags |= FLAG_ZERO;
    if (cpu->A & 0x80)
        cpu->flags |= FLAG_NEGATIVE;
    return CPU_OK;
}

static int op_ldx(CPU *cpu, uint8_t mode, uint8_t operand) {
    switch (mode) {
        case MODE_IMMEDIAT:
            cpu->X = operand;
            break;
        case MODE_ABSOLUTE:
            cpu->X = cpu->memory[operand];
            break;
        case MODE_INDIRECT:
            cpu->X = cpu->memory[cpu->memory[operand]];
            break;
        default:
            cpu->flags |= FLAG_ERROR;
            return CPU_ERROR;
    }

    // Update flags Zero et Sign
    cpu->flags &= ~(FLAG_ZERO | FLAG_NEGATIVE);
    if (cpu->X == 0)
        cpu->flags |= FLAG_ZERO;
    if (cpu->X & 0x80)
        cpu->flags |= FLAG_NEGATIVE;
    return CPU_OK;
}

static inline int op_sta(CPU *cpu, uint8_t mode, uint8_t operand) {
    switch (mode) {
        case MODE_ABSOLUTE:
            cpu->memory[operand] = cpu->A;
            break;
        case MODE_INDEXED_X:
            cpu->memory[(operand + cpu->X) & 0xFF] = cpu->A;
            break;
        case MODE_INDIRECT:
            cpu->memory[cpu->memory[operand]] = cpu->A;
            break;
        case MODE_INDIRECT_INDEXED_X:
            cpu->memory[cpu->memory[(operand + cpu->X) & 0xFF]] = cpu->A;
            break;
        default:
            cpu->flags |= FLAG_ERROR;
            return CPU_ERROR;
    }
    return CPU_OK;
}

static int op_stx(CPU *cpu, uint8_t mode, uint8_t operand) {
    switch (mode) {
        case MODE_ABSOLUTE:
            cpu->memory[operand] = cpu->X;
            break;
        case MODE_INDIRECT:
            cpu->memory[cpu->memory[operand]] = cpu->X;
            break;
        default:
            cpu->flags |= FLAG_ERROR;
            return CPU_ERROR;
    }
    return CPU_OK;
}

static int op_add(CPU *cpu, uint8_t mode, uint8_t operand) {
    uint8_t value = 0;
    switch (mode) {
        case MODE_IMMEDIAT:
            value = operand;
            break;
        case MODE_ABSOLUTE:
            value = cpu->memory[operand];
            break;
        case MODE_INDEXED_X:
            value = cpu->memory[(operand + cpu->X) & 0xFF];
            break;
        case MODE_INDIRECT:
            value = cpu->memory[cpu->memory[operand]];
            break;
        case MODE_INDIRECT_INDEXED_X:
            value = cpu->memory[cpu->memory[(operand + cpu->X) & 0xFF]];
            break;
        default:
            cpu->flags |= FLAG_ERROR;
            return CPU_ERROR;
    }

    uint8_t original_A = cpu->A;  // Save original A value for overflow check
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
    if (((original_A ^ value) & 0x80) == 0 && ((original_A ^ cpu->A) & 0x80) != 0)
        cpu->flags |= FLAG_OVERFLOW;
    
    return CPU_OK;
}

static int op_sub(CPU *cpu, uint8_t mode, uint8_t operand) {
    uint8_t value = 0;
    switch (mode) {
        case MODE_IMMEDIAT:
            value = operand;
            break;
        case MODE_ABSOLUTE:
            value = cpu->memory[operand];
            break;
        case MODE_INDEXED_X:
            value = cpu->memory[(operand + cpu->X) & 0xFF];
            break;
        case MODE_INDIRECT:
            value = cpu->memory[cpu->memory[operand]];
            break;
        case MODE_INDIRECT_INDEXED_X:
            value = cpu->memory[cpu->memory[(operand + cpu->X) & 0xFF]];
            break;
        default:
            cpu->flags |= FLAG_ERROR;
            return CPU_ERROR;
    }

    uint8_t original_A = cpu->A;  // Save original A value for overflow check
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

    // Overflow pour soustraction: operands have different signs and result has different sign from minuend
    if (((original_A ^ value) & 0x80) != 0 && ((original_A ^ cpu->A) & 0x80) != 0)
        cpu->flags |= FLAG_OVERFLOW;
    
    return CPU_OK;
}

static int op_and(CPU *cpu, uint8_t mode, uint8_t operand) {
    uint8_t value;
    switch (mode) {
        case MODE_IMMEDIAT:
            value = operand;
            break;
        case MODE_ABSOLUTE:
            value = cpu->memory[operand];
            break;
        case MODE_INDEXED_X:
            value = cpu->memory[(operand + cpu->X) & 0xFF];
            break;
        case MODE_INDIRECT:
            value = cpu->memory[cpu->memory[operand]];
            break;
        case MODE_INDIRECT_INDEXED_X:
            value = cpu->memory[cpu->memory[(operand + cpu->X) & 0xFF]];
            break;
        default:
            cpu->flags |= FLAG_ERROR;
            return CPU_ERROR;
    }

    cpu->A &= value;

    // Update flags Zero et Sign
    cpu->flags &= ~(FLAG_ZERO | FLAG_NEGATIVE);
    if (cpu->A == 0)
        cpu->flags |= FLAG_ZERO;
    if (cpu->A & 0x80)
        cpu->flags |= FLAG_NEGATIVE;
    return CPU_OK;
}

static int op_xor(CPU *cpu, uint8_t mode, uint8_t operand) {
    uint8_t value;
    switch (mode) {
        case MODE_IMMEDIAT:
            value = operand;
            break;
        case MODE_ABSOLUTE:
            value = cpu->memory[operand];
            break;
        case MODE_INDEXED_X:
            value = cpu->memory[(operand + cpu->X) & 0xFF];
            break;
        case MODE_INDIRECT:
            value = cpu->memory[cpu->memory[operand]];
            break;
        case MODE_INDIRECT_INDEXED_X:
            value = cpu->memory[cpu->memory[(operand + cpu->X) & 0xFF]];
            break;
        default:
            cpu->flags |= FLAG_ERROR;
            return CPU_ERROR;
    }

    cpu->A ^= value;

    cpu->flags &= ~(FLAG_ZERO | FLAG_NEGATIVE);
    if (cpu->A == 0)
        cpu->flags |= FLAG_ZERO;
    if (cpu->A & 0x80)
        cpu->flags |= FLAG_NEGATIVE;
    
    return CPU_OK;
}

static int op_or(CPU *cpu, uint8_t mode, uint8_t operand) {
    uint8_t value = 0;
    switch (mode) {
        case MODE_IMMEDIAT:
            value = operand;
            break;
        case MODE_ABSOLUTE:
            value = cpu->memory[operand];
            break;
        case MODE_INDEXED_X:
            value = cpu->memory[(operand + cpu->X) & 0xFF];
            break;
        case MODE_INDIRECT:
            value = cpu->memory[cpu->memory[operand]];
            break;
        case MODE_INDIRECT_INDEXED_X:
            value = cpu->memory[cpu->memory[(operand + cpu->X) & 0xFF]];
            break;
        default:
            cpu->flags |= FLAG_ERROR;
            return CPU_ERROR;
    }

    cpu->A |= value;

    cpu->flags &= ~(FLAG_ZERO | FLAG_NEGATIVE);
    if (cpu->A == 0)
        cpu->flags |= FLAG_ZERO;
    if (cpu->A & 0x80)
        cpu->flags |= FLAG_NEGATIVE;
    
    return CPU_OK;
}

static int op_branch(CPU *cpu, uint8_t condition, uint8_t address) {
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
    
    return CPU_OK;
}

static int op_cmp(CPU *cpu, uint8_t mode, uint8_t operand) {
    uint8_t value = 0;
    switch (mode) {
        case MODE_IMMEDIAT:
            value = operand;
            break;
        case MODE_ABSOLUTE:
            value = cpu->memory[operand];
            break;
        case MODE_INDEXED_X:
            value = cpu->memory[(operand + cpu->X) & 0xFF];
            break;
        case MODE_INDIRECT:
            value = cpu->memory[cpu->memory[operand]];
            break;
        case MODE_INDIRECT_INDEXED_X:
            value = cpu->memory[cpu->memory[(operand + cpu->X) & 0xFF]];
            break;
        default:
            cpu->flags |= FLAG_ERROR;
            return CPU_ERROR;
    }

    // Perform comparison (A - value) without storing result
    uint8_t original_A = cpu->A;
    uint16_t result = cpu->A - value;
    uint8_t result_byte = result & 0xFF;

    // Update flags
    cpu->flags &= ~(FLAG_ZERO | FLAG_NEGATIVE | FLAG_CARRY | FLAG_OVERFLOW);

    if (result_byte == 0)
        cpu->flags |= FLAG_ZERO;
    if (result_byte & 0x80)
        cpu->flags |= FLAG_NEGATIVE;
    if (result < 0x100)  // No borrow
        cpu->flags |= FLAG_CARRY;

    // Overflow for comparison: operands have different signs and result has different sign from minuend
    if (((original_A ^ value) & 0x80) != 0 && ((original_A ^ result_byte) & 0x80) != 0)
        cpu->flags |= FLAG_OVERFLOW;
    
    return CPU_OK;
}

static int op_cpx(CPU *cpu, uint8_t mode, uint8_t operand) {
    uint8_t value = 0;
    switch (mode) {
        case MODE_IMMEDIAT:
            value = operand;
            break;
        case MODE_ABSOLUTE:
            value = cpu->memory[operand];
            break;
        case MODE_INDIRECT:
            value = cpu->memory[cpu->memory[operand]];
            break;
        default:
            cpu->flags |= FLAG_ERROR;
            return CPU_ERROR;
    }

    // Perform comparison (X - value) without storing result
    uint8_t original_X = cpu->X;
    uint16_t result = cpu->X - value;
    uint8_t result_byte = result & 0xFF;

    // Update flags
    cpu->flags &= ~(FLAG_ZERO | FLAG_NEGATIVE | FLAG_CARRY | FLAG_OVERFLOW);

    if (result_byte == 0)
        cpu->flags |= FLAG_ZERO;
    if (result_byte & 0x80)
        cpu->flags |= FLAG_NEGATIVE;
    if (result < 0x100)  // No borrow
        cpu->flags |= FLAG_CARRY;

    // Overflow for comparison: operands have different signs and result has different sign from minuend
    if (((original_X ^ value) & 0x80) != 0 && ((original_X ^ result_byte) & 0x80) != 0)
        cpu->flags |= FLAG_OVERFLOW;
    
    return CPU_OK;
}

static int op_push(CPU *cpu, uint8_t mode, uint8_t operand) {
    (void)mode; (void)operand;  // PUSH pousse toujours A

    if (cpu->SP < (STACK_BASE - STACK_SIZE + 1)) {  // Stack overflow check
        cpu->flags |= FLAG_ERROR;
        return CPU_ERROR;
    }

    cpu->memory[cpu->SP] = cpu->A;
    cpu->SP--;
    return CPU_OK;
}

static int op_pop(CPU *cpu, uint8_t mode, uint8_t operand) {
    (void)mode; (void)operand;  // POP récupère toujours dans A

    if (cpu->SP >= STACK_BASE) {  // Stack underflow check
        cpu->flags |= FLAG_ERROR;
        return CPU_ERROR;
    }

    cpu->SP++;
    cpu->A = cpu->memory[cpu->SP];

    // Update flags Zero et Sign
    cpu->flags &= ~(FLAG_ZERO | FLAG_NEGATIVE);
    if (cpu->A == 0)
        cpu->flags |= FLAG_ZERO;
    if (cpu->A & 0x80)
        cpu->flags |= FLAG_NEGATIVE;
    return CPU_OK;
}

static inline int op_halt(CPU *cpu, uint8_t mode, uint8_t operand) {
    (void)cpu; (void)mode; (void)operand;
    return CPU_HALT;
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
    [OPCODE_CMP]  = op_cmp,
    [OPCODE_CPX]  = op_cpx,
    [OPCODE_HALT] = op_halt,
};

// CPU initialization
static void initCPU(CPU *cpu) {
    __builtin_memset(cpu, 0, sizeof(CPU));
    cpu->SP = STACK_BASE;
}

// cpu_step
static inline __attribute__((always_inline)) int cpu_step(CPU *cpu) {
    uint8_t opcode;
    uint8_t mode;
    uint8_t operand;

    // Table de labels pour dispatch rapide (computed goto)
    static void *dispatch_table[16] = {
        &&op_nop_lbl,   // 0x00
        &&op_lda_lbl,   // 0x01
        &&op_ldx_lbl,   // 0x02
        &&op_sta_lbl,   // 0x03
        &&op_stx_lbl,   // 0x04
        &&op_add_lbl,   // 0x05
        &&op_sub_lbl,   // 0x06
        &&op_xor_lbl,   // 0x07
        &&op_and_lbl,   // 0x08
        &&op_or_lbl,    // 0x09
        &&op_branch_lbl,// 0x0A
        &&op_pop_lbl,   // 0x0B
        &&op_push_lbl,  // 0x0C
        &&op_cmp_lbl,   // 0x0D
        &&op_cpx_lbl,   // 0x0E
        &&op_halt_lbl   // 0x0F
    };

    opcode = cpu->memory[cpu->PC++];
    if (opcode >= 16) goto invalid_lbl;

    // Check HALT
    if (opcode == OPCODE_HALT) {
        return CPU_HALT;  // Signal HALT
    }

    // Lecture groupée mode/operand
    mode = cpu->memory[cpu->PC++];
    operand = cpu->memory[cpu->PC++];

    goto *dispatch_table[opcode];

invalid_lbl:
        // Opcode invalide
        return CPU_ERROR;
op_nop_lbl:
    op_nop(cpu, mode, operand);
    return CPU_OK;

op_lda_lbl:
    return op_lda(cpu, mode, operand);

op_ldx_lbl:
    return op_ldx(cpu, mode, operand);

op_sta_lbl:
    return op_sta(cpu, mode, operand);

op_stx_lbl:
    return op_stx(cpu, mode, operand);

op_add_lbl:
    return op_add(cpu, mode, operand);

op_sub_lbl:
    return op_sub(cpu, mode, operand);

op_xor_lbl:
    return op_xor(cpu, mode, operand);

op_and_lbl:
    return op_and(cpu, mode, operand);

op_or_lbl:
    return op_or(cpu, mode, operand);

op_branch_lbl:
    return op_branch(cpu, mode, operand);

op_pop_lbl:
    return op_pop(cpu, mode, operand);

op_push_lbl:
    return op_push(cpu, mode, operand);

op_cmp_lbl:
    return op_cmp(cpu, mode, operand);

op_cpx_lbl:
    return op_cpx(cpu, mode, operand);

op_halt_lbl:
    op_halt(cpu, mode, operand);
    return CPU_HALT;

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
