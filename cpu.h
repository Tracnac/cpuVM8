#ifndef CPU_H
#define CPU_H

#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>

// Easy memory mapping with 256 bytes total
/*
 * 0x00-0xEF : Code (240 bytes)
 * 0xF0-0xFF : Stack (16 bytes)
 */

// Retour de cpu_step
#define CPU_ERROR -1
#define CPU_OK 0
#define CPU_HALT 1

// Memory layout
#define CODE_BASE 0x00
#define STACK_BASE 0xFF
#define STACK_SIZE 16
#define MAX_MEMORY_SIZE 256

// Registers
enum {
  REG_A = 0, // Accumulator
  REG_X,     // Index register
  REG_PC,    // Program Counter
  REG_SP,    // Stack Pointer
};

// Flags
enum {
  FLAG_CARRY = 1 << 0,    // Carry C
  FLAG_ZERO = 1 << 1,     // Zero Z
  FLAG_NEGATIVE = 1 << 2, // Negative N
  FLAG_OVERFLOW = 1 << 3, // Overflow O
  FLAG_ERROR = 1 << 4     // Error (invalid instruction, memory access, etc.)
};

// Opcodes
enum {
  OPCODE_NOP = 0x00, // No Operation
  OPCODE_LDA,        // Load Accumulator
  OPCODE_LDX,        // Load Register X
  OPCODE_STA,        // Store Accumulator
  OPCODE_STX,        // Store Register X
  OPCODE_ADD,        // Add
  OPCODE_SUB,        // Subtract
  OPCODE_XOR,        // Exclusive OR
  OPCODE_AND,        // AND
  OPCODE_OR,         // OR
  OPCODE_B,          // B NE, B EQ, B AL ...
  OPCODE_POP,        // Pop from Stack
  OPCODE_PUSH,       // Push to Stack
  OPCODE_CMP,        // Compare (sets flags without storing result)
  OPCODE_CPX,        // Compare X register (sets flags without storing result)
  OPCODE_HALT,       // Halt
  OPCODE_ROR,        // Rotate Right
  OPCODE_ROL,        // Rotate Left
  OPCODE_SHR,        // Shift Right
  OPCODE_SHL,        // Shift Left
  OPCODE_INX,        // Increment X
  OPCODE_DEX,        // Decrement X
};

// Addressing modes
enum {
  MODE_IMMEDIAT = 0x00, // Immediate:  #value
  MODE_ABSOLUTE,        // Absolute:   address
  MODE_ABSOLUTE_X,      // Indexed:    address,X
  MODE_INDIRECT,        // Indirect:   [$address]
  MODE_INDIRECT_X,      // Indirect Indexed: [$address,X]
  MODE_REGISTER,        // REGISTERS (Not used yet)
};

// Branch conditions (byte après OPCODE_B)
enum {
  COND_AL = 0x00, // Always (unconditional jump)
  COND_EQ,        // Equal (Z=1)
  COND_NE,        // Not Equal (Z=0)
  COND_CS,        // Carry Set (C=1)
  COND_CC,        // Carry Clear (C=0)
  COND_MI,        // Minus/Negative (N=1)
  COND_PL,        // Plus/Positive (N=0)
};

// CPU
typedef struct {
  uint8_t A;                        // Accumulator
  uint8_t X;                        // Index register
  uint8_t PC;                       // Program Counter
  uint8_t SP;                       // Stack Pointer
  uint8_t flags;                    // Status flags·
  uint8_t memory[MAX_MEMORY_SIZE];  // 256 bytes of memory
} CPU __attribute__((aligned(64))); // Align to cache line size for performance

// Define a function pointer type for opcode handlers
typedef int (*opcode_handler)(CPU *, uint8_t, uint8_t);

// ============================================================================
// OPCODE HANDLERS
// ============================================================================

static inline int op_nop(CPU *cpu, uint8_t mode, uint8_t operand) {
  (void)cpu;
  (void)mode;
  (void)operand;
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
  case MODE_ABSOLUTE_X:
    cpu->A = cpu->memory[(operand + cpu->X) & 0xFF];
    break;
  case MODE_INDIRECT:
    cpu->A = cpu->memory[cpu->memory[operand]];
    break;
  case MODE_INDIRECT_X:
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
  case MODE_ABSOLUTE_X:
    cpu->memory[(operand + cpu->X) & 0xFF] = cpu->A;
    break;
  case MODE_INDIRECT:
    cpu->memory[cpu->memory[operand]] = cpu->A;
    break;
  case MODE_INDIRECT_X:
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
  case MODE_ABSOLUTE_X:
    value = cpu->memory[(operand + cpu->X) & 0xFF];
    break;
  case MODE_INDIRECT:
    value = cpu->memory[cpu->memory[operand]];
    break;
  case MODE_INDIRECT_X:
    value = cpu->memory[cpu->memory[(operand + cpu->X) & 0xFF]];
    break;
  default:
    cpu->flags |= FLAG_ERROR;
    return CPU_ERROR;
  }

  uint8_t original_A = cpu->A; // Save original A value for overflow check
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
  case MODE_ABSOLUTE_X:
    value = cpu->memory[(operand + cpu->X) & 0xFF];
    break;
  case MODE_INDIRECT:
    value = cpu->memory[cpu->memory[operand]];
    break;
  case MODE_INDIRECT_X:
    value = cpu->memory[cpu->memory[(operand + cpu->X) & 0xFF]];
    break;
  default:
    cpu->flags |= FLAG_ERROR;
    return CPU_ERROR;
  }

  uint8_t original_A = cpu->A; // Save original A value for overflow check
  uint16_t result = cpu->A - value;
  cpu->A = result & 0xFF;

  // Update flags
  cpu->flags &= ~(FLAG_ZERO | FLAG_NEGATIVE | FLAG_CARRY | FLAG_OVERFLOW);

  if (cpu->A == 0)
    cpu->flags |= FLAG_ZERO;
  if (cpu->A & 0x80)
    cpu->flags |= FLAG_NEGATIVE;
  if (result < 0x100) // Pas de borrow
    cpu->flags |= FLAG_CARRY;

  // Overflow pour soustraction: operands have different signs and result has
  // different sign from minuend
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
  case MODE_ABSOLUTE_X:
    value = cpu->memory[(operand + cpu->X) & 0xFF];
    break;
  case MODE_INDIRECT:
    value = cpu->memory[cpu->memory[operand]];
    break;
  case MODE_INDIRECT_X:
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
  case MODE_ABSOLUTE_X:
    value = cpu->memory[(operand + cpu->X) & 0xFF];
    break;
  case MODE_INDIRECT:
    value = cpu->memory[cpu->memory[operand]];
    break;
  case MODE_INDIRECT_X:
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
  case MODE_ABSOLUTE_X:
    value = cpu->memory[(operand + cpu->X) & 0xFF];
    break;
  case MODE_INDIRECT:
    value = cpu->memory[cpu->memory[operand]];
    break;
  case MODE_INDIRECT_X:
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

  switch (condition) {
  case COND_AL: // Always
    should_branch = 1;
    break;
  case COND_EQ: // Equal (Z=1)
    should_branch = (cpu->flags & FLAG_ZERO) != 0;
    break;
  case COND_NE: // Not Equal (Z=0)
    should_branch = (cpu->flags & FLAG_ZERO) == 0;
    break;
  case COND_CS: // Carry Set (C=1)
    should_branch = (cpu->flags & FLAG_CARRY) != 0;
    break;
  case COND_CC: // Carry Clear (C=0)
    should_branch = (cpu->flags & FLAG_CARRY) == 0;
    break;
  case COND_MI: // Minus/Negative (N=1)
    should_branch = (cpu->flags & FLAG_NEGATIVE) != 0;
    break;
  case COND_PL: // Plus/Positive (N=0)
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
  case MODE_ABSOLUTE_X:
    value = cpu->memory[(operand + cpu->X) & 0xFF];
    break;
  case MODE_INDIRECT:
    value = cpu->memory[cpu->memory[operand]];
    break;
  case MODE_INDIRECT_X:
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
  if (result < 0x100) // No borrow
    cpu->flags |= FLAG_CARRY;

  // Overflow for comparison: operands have different signs and result has
  // different sign from minuend
  if (((original_A ^ value) & 0x80) != 0 &&
      ((original_A ^ result_byte) & 0x80) != 0)
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
  if (result < 0x100) // No borrow
    cpu->flags |= FLAG_CARRY;

  // Overflow for comparison: operands have different signs and result has
  // different sign from minuend
  if (((original_X ^ value) & 0x80) != 0 &&
      ((original_X ^ result_byte) & 0x80) != 0)
    cpu->flags |= FLAG_OVERFLOW;

  return CPU_OK;
}

static int op_push(CPU *cpu, uint8_t mode, uint8_t operand) {
  (void)mode;
  (void)operand; // PUSH A

  if (cpu->SP < (STACK_BASE - STACK_SIZE + 1)) { // Stack overflow check
    cpu->flags |= FLAG_ERROR;
    return CPU_ERROR;
  }

  cpu->memory[cpu->SP] = cpu->A;
  cpu->SP--;
  return CPU_OK;
}

static int op_pop(CPU *cpu, uint8_t mode, uint8_t operand) {
  (void)mode;
  (void)operand; // POP A

  if (cpu->SP >= STACK_BASE) { // Stack underflow check
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
  (void)cpu;
  (void)mode;
  (void)operand;
  return CPU_HALT;
}

static int op_ror(CPU *cpu, uint8_t mode, uint8_t operand) {
  uint8_t value = 0;
  uint8_t *target = NULL;

  switch (mode) {
  case MODE_REGISTER:
    target = &cpu->A;
    value = cpu->A;
    break;
  case MODE_ABSOLUTE:
    target = &cpu->memory[operand];
    value = cpu->memory[operand];
    break;
  case MODE_ABSOLUTE_X:
    target = &cpu->memory[(operand + cpu->X) & 0xFF];
    value = cpu->memory[(operand + cpu->X) & 0xFF];
    break;
  case MODE_INDIRECT:
    target = &cpu->memory[cpu->memory[operand]];
    value = cpu->memory[cpu->memory[operand]];
    break;
  case MODE_INDIRECT_X:
    target = &cpu->memory[cpu->memory[(operand + cpu->X) & 0xFF]];
    value = cpu->memory[cpu->memory[(operand + cpu->X) & 0xFF]];
    break;
  default:
    cpu->flags |= FLAG_ERROR;
    return CPU_ERROR;
  }

  uint8_t old_carry = (cpu->flags & FLAG_CARRY) ? 1 : 0;
  uint8_t new_carry = value & 1;

  uint8_t result = (uint8_t)((value >> 1) | (old_carry << 7));
  *target = result;

  cpu->flags &= ~(FLAG_ZERO | FLAG_NEGATIVE | FLAG_CARRY);
  if (result == 0)
    cpu->flags |= FLAG_ZERO;
  if (result & 0x80)
    cpu->flags |= FLAG_NEGATIVE;
  if (new_carry)
    cpu->flags |= FLAG_CARRY;
  return CPU_OK;
}

static int op_rol(CPU *cpu, uint8_t mode, uint8_t operand) {
  uint8_t value = 0;
  uint8_t *target = NULL;

  switch (mode) {
  case MODE_REGISTER:
    target = &cpu->A;
    value = cpu->A;
    break;
  case MODE_ABSOLUTE:
    target = &cpu->memory[operand];
    value = cpu->memory[operand];
    break;
  case MODE_ABSOLUTE_X:
    target = &cpu->memory[(operand + cpu->X) & 0xFF];
    value = cpu->memory[(operand + cpu->X) & 0xFF];
    break;
  case MODE_INDIRECT:
    target = &cpu->memory[cpu->memory[operand]];
    value = cpu->memory[cpu->memory[operand]];
    break;
  case MODE_INDIRECT_X:
    target = &cpu->memory[cpu->memory[(operand + cpu->X) & 0xFF]];
    value = cpu->memory[cpu->memory[(operand + cpu->X) & 0xFF]];
    break;
  default:
    cpu->flags |= FLAG_ERROR;
    return CPU_ERROR;
  }

  uint8_t old_carry = (cpu->flags & FLAG_CARRY) ? 1 : 0;
  uint8_t new_carry = (value & 0x80) ? 1 : 0;

  uint8_t result = (uint8_t)((value << 1) | old_carry);
  *target = result;

  cpu->flags &= ~(FLAG_ZERO | FLAG_NEGATIVE | FLAG_CARRY);
  if (result == 0)
    cpu->flags |= FLAG_ZERO;
  if (result & 0x80)
    cpu->flags |= FLAG_NEGATIVE;
  if (new_carry)
    cpu->flags |= FLAG_CARRY;
  return CPU_OK;
}

static int op_shr(CPU *cpu, uint8_t mode, uint8_t operand) {
  uint8_t value = 0;
  uint8_t *target = NULL;

  switch (mode) {
  case MODE_REGISTER:
    target = &cpu->A;
    value = cpu->A;
    break;
  case MODE_ABSOLUTE:
    target = &cpu->memory[operand];
    value = cpu->memory[operand];
    break;
  case MODE_ABSOLUTE_X:
    target = &cpu->memory[(operand + cpu->X) & 0xFF];
    value = cpu->memory[(operand + cpu->X) & 0xFF];
    break;
  case MODE_INDIRECT:
    target = &cpu->memory[cpu->memory[operand]];
    value = cpu->memory[cpu->memory[operand]];
    break;
  case MODE_INDIRECT_X:
    target = &cpu->memory[cpu->memory[(operand + cpu->X) & 0xFF]];
    value = cpu->memory[cpu->memory[(operand + cpu->X) & 0xFF]];
    break;
  default:
    cpu->flags |= FLAG_ERROR;
    return CPU_ERROR;
  }

  uint8_t new_carry = value & 1;
  uint8_t result = value >> 1;
  *target = result;

  cpu->flags &= ~(FLAG_ZERO | FLAG_NEGATIVE | FLAG_CARRY);
  if (result == 0)
    cpu->flags |= FLAG_ZERO;
  if (result & 0x80)
    cpu->flags |= FLAG_NEGATIVE;
  if (new_carry)
    cpu->flags |= FLAG_CARRY;
  return CPU_OK;
}

static int op_shl(CPU *cpu, uint8_t mode, uint8_t operand) {
  uint8_t value = 0;
  uint8_t *target = NULL;

  switch (mode) {
  case MODE_REGISTER:
    target = &cpu->A;
    value = cpu->A;
    break;
  case MODE_ABSOLUTE:
    target = &cpu->memory[operand];
    value = cpu->memory[operand];
    break;
  case MODE_ABSOLUTE_X:
    target = &cpu->memory[(operand + cpu->X) & 0xFF];
    value = cpu->memory[(operand + cpu->X) & 0xFF];
    break;
  case MODE_INDIRECT:
    target = &cpu->memory[cpu->memory[operand]];
    value = cpu->memory[cpu->memory[operand]];
    break;
  case MODE_INDIRECT_X:
    target = &cpu->memory[cpu->memory[(operand + cpu->X) & 0xFF]];
    value = cpu->memory[cpu->memory[(operand + cpu->X) & 0xFF]];
    break;
  default:
    cpu->flags |= FLAG_ERROR;
    return CPU_ERROR;
  }

  uint8_t new_carry = (value & 0x80) ? 1 : 0;
  uint8_t result = (uint8_t)(value << 1);
  *target = result;

  cpu->flags &= ~(FLAG_ZERO | FLAG_NEGATIVE | FLAG_CARRY);
  if (result == 0)
    cpu->flags |= FLAG_ZERO;
  if (result & 0x80)
    cpu->flags |= FLAG_NEGATIVE;
  if (new_carry)
    cpu->flags |= FLAG_CARRY;
  return CPU_OK;
}

static int op_inx(CPU *cpu, uint8_t mode, uint8_t operand) {
  (void)mode;
  (void)operand; // INX operates only on X register

  cpu->X++;

  cpu->flags &= ~(FLAG_ZERO | FLAG_NEGATIVE);
  if (cpu->X == 0)
    cpu->flags |= FLAG_ZERO;
  if (cpu->X & 0x80)
    cpu->flags |= FLAG_NEGATIVE;
  return CPU_OK;
}

static int op_dex(CPU *cpu, uint8_t mode, uint8_t operand) {
  (void)mode;
  (void)operand; // DEX operates only on X register

  cpu->X--;

  cpu->flags &= ~(FLAG_ZERO | FLAG_NEGATIVE);
  if (cpu->X == 0)
    cpu->flags |= FLAG_ZERO;
  if (cpu->X & 0x80)
    cpu->flags |= FLAG_NEGATIVE;
  return CPU_OK;
}

// Table de dispatch
static const opcode_handler handlers[22] = {
    [OPCODE_NOP] = op_nop,   [OPCODE_LDA] = op_lda,  [OPCODE_LDX] = op_ldx,
    [OPCODE_ADD] = op_add,   [OPCODE_SUB] = op_sub,  [OPCODE_XOR] = op_xor,
    [OPCODE_STA] = op_sta,   [OPCODE_STX] = op_stx,  [OPCODE_AND] = op_and,
    [OPCODE_OR] = op_or,     [OPCODE_B] = op_branch, [OPCODE_POP] = op_pop,
    [OPCODE_PUSH] = op_push, [OPCODE_CMP] = op_cmp,  [OPCODE_CPX] = op_cpx,
    [OPCODE_HALT] = op_halt, [OPCODE_ROR] = op_ror,  [OPCODE_ROL] = op_rol,
    [OPCODE_SHR] = op_shr,   [OPCODE_SHL] = op_shl,  [OPCODE_INX] = op_inx,
    [OPCODE_DEX] = op_dex,
};

// CPU initialization
static inline void initCPU(CPU *cpu) {
  __builtin_memset(cpu, 0, sizeof(CPU));
  cpu->SP = STACK_BASE;
}

// cpu_step
static inline int cpu_step(CPU *cpu) {

  // Fetch
  uint8_t opcode = cpu->memory[cpu->PC++];
  if (opcode >= 22 || !handlers[opcode])
    return CPU_ERROR;

  // Check HALT
  if (opcode == OPCODE_HALT)
    return CPU_HALT; // Signal HALT

  // Fetch mode et operand
  uint8_t mode = cpu->memory[cpu->PC++];
  uint8_t operand = cpu->memory[cpu->PC++];

  // Execute
  return handlers[opcode](cpu, mode, operand);
}

static inline void cpu_run(CPU *cpu) {
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
  }
}

#endif // CPU_H
