#ifndef CPU_H
#define CPU_H

#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>

/* Packing helpers: 5-bit opcode + 3-bit mode */
#define PACK_INST_BYTE(opcode, mode)                                           \
  ((uint8_t)((((mode) & 0x7) << 5) | ((opcode) & 0x1F)))
#define UNPACK_OPCODE(b) ((uint8_t)((b) & 0x1F))
#define UNPACK_MODE(b) ((uint8_t)(((b) >> 5) & 0x07))

/* Performance optimization macros */
#define LIKELY(x) __builtin_expect(!!(x), 1)
#define UNLIKELY(x) __builtin_expect(!!(x), 0)

/* Branchless flag update helpers for better performance */
#define UPDATE_ZN_FLAGS(cpu, value)                                            \
  do {                                                                         \
    (cpu)->flags &= ~(FLAG_ZERO | FLAG_NEGATIVE);                              \
    (cpu)->flags |= ((value) == 0) ? FLAG_ZERO : 0;                            \
    (cpu)->flags |= ((value) & 0x80) ? FLAG_NEGATIVE : 0;                      \
  } while (0)

#define UPDATE_ZNC_FLAGS(cpu, value, carry_condition)                          \
  do {                                                                         \
    (cpu)->flags &= ~(FLAG_ZERO | FLAG_NEGATIVE | FLAG_CARRY);                 \
    (cpu)->flags |= ((value) == 0) ? FLAG_ZERO : 0;                            \
    (cpu)->flags |= ((value) & 0x80) ? FLAG_NEGATIVE : 0;                      \
    (cpu)->flags |= (carry_condition) ? FLAG_CARRY : 0;                        \
  } while (0)

// Easy memory mapping with 256 bytes total
/*
 * 0x00-0xEF : Code (240 bytes)
 * 0xF0-0xFF : Stack (16 bytes)
 */

// Legacy constants (kept for compatibility)
#define CPU_OK 0
#define CPU_HALTED -1

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
  FLAG_HALTED =
      1 << 4 // Halted state, Error (invalid instruction, memory access, etc.)
};

// Opcodes
enum {
  OPCODE_NOP = 0x00, // No Operation
  OPCODE_LDA,        // Load Accumulator
  OPCODE_LDX,        // Load Register X
  OPCODE_STA,        // Store Accumulator
  OPCODE_STX,        // Store Register X
  OPCODE_B,          // B NE, B EQ, B AL ...
  OPCODE_ADD,        // Add
  OPCODE_SUB,        // Subtract
  OPCODE_XOR,        // Exclusive OR
  OPCODE_AND,        // AND
  OPCODE_OR,         // OR
  OPCODE_POP,        // Pop from Stack
  OPCODE_PUSH,       // Push to Stack
  OPCODE_CMP,        // Compare (sets flags without storing result)
  OPCODE_CPX,        // Compare X register (sets flags without storing result)
  OPCODE_ROR,        // Rotate Right
  OPCODE_ROL,        // Rotate Left
  OPCODE_SHR,        // Shift Right
  OPCODE_SHL,        // Shift Left
  OPCODE_INX,        // Increment X
  OPCODE_DEX,        // Decrement X
  OPCODE_HALT,       // Halt

  OPCODE_COUNT // Number of opcodes (DON'T REMOVE)
};
_Static_assert(OPCODE_COUNT <= 256, "too many opcodes");
_Static_assert(OPCODE_COUNT <= 32, "too many opcodes for packed format");

// Addressing modes
enum {
  MODE_IMMEDIAT = 0x00, // Immediate:  #value
  MODE_ABSOLUTE,        // Absolute:   address
  MODE_ABSOLUTE_X,      // Indexed:    address,X
  MODE_INDIRECT,        // Indirect:   [$address]
  MODE_INDIRECT_X,      // Indirect Indexed: [$address,X]
  MODE_REGISTER,        // REGISTERS (Not used yet)

  MODE_COUNT // Number of addressing modes (DON'T REMOVE)
};
_Static_assert(MODE_COUNT <= 256, "too many addressing modes");
_Static_assert(MODE_COUNT <= 8, "too many addressing modes for packed format");

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

/* Fast address calculation helper used by STA, STX */
static inline uint8_t get_effective_address(const CPU *cpu, uint8_t mode,
                                            uint8_t operand) {
  switch (mode) {
  case MODE_ABSOLUTE:
    return operand;
  case MODE_ABSOLUTE_X:
    return (operand + cpu->X) & 0xFF;
  case MODE_INDIRECT:
    return cpu->memory[operand];
  case MODE_INDIRECT_X:
    return cpu->memory[(operand + cpu->X) & 0xFF];
  }
  __builtin_unreachable();  // Unreachable - mode validated in cpu_step
}

/* Fast value retrieval helper for instructions that read memory */
static inline uint8_t get_operand_value(const CPU *cpu, uint8_t mode,
                                        uint8_t operand) {
  switch (mode) {
  case MODE_IMMEDIAT:
    return operand;
  case MODE_ABSOLUTE:
    return cpu->memory[operand];
  case MODE_ABSOLUTE_X:
    return cpu->memory[(operand + cpu->X) & 0xFF];
  case MODE_INDIRECT:
    return cpu->memory[cpu->memory[operand]];
  case MODE_INDIRECT_X:
    return cpu->memory[cpu->memory[(operand + cpu->X) & 0xFF]];
  }
  __builtin_unreachable();  // Unreachable - mode validated in cpu_step
}

// Define a function pointer type for opcode handlers
typedef void (*opcode_handler)(CPU *, uint8_t, uint8_t);
/* CPU Step function pointer type */
typedef int (*cpu_step_fn)(CPU *);

// ============================================================================
// OPCODE HANDLERS - OPTIMIZED FOR PERFORMANCE
// ============================================================================

static inline void op_nop(CPU *cpu, uint8_t mode, uint8_t operand) {
  (void)cpu;
  (void)mode;
  (void)operand;
}

static inline void op_lda(CPU *cpu, uint8_t mode, uint8_t operand) {
  cpu->A = get_operand_value(cpu, mode, operand);
  UPDATE_ZN_FLAGS(cpu, cpu->A);
}

static void op_ldx(CPU *cpu, uint8_t mode, uint8_t operand) {
  cpu->X = get_operand_value(cpu, mode, operand);
  UPDATE_ZN_FLAGS(cpu, cpu->X);
}

static inline void op_sta(CPU *cpu, uint8_t mode, uint8_t operand) {
  uint8_t address = get_effective_address(cpu, mode, operand);
  cpu->memory[address] = cpu->A;
}

static void op_stx(CPU *cpu, uint8_t mode, uint8_t operand) {
  uint8_t address = get_effective_address(cpu, mode, operand);
  cpu->memory[address] = cpu->X;
}

static void op_add(CPU *cpu, uint8_t mode, uint8_t operand) {
  uint8_t value = get_operand_value(cpu, mode, operand);

  uint8_t original_A = cpu->A;
  uint16_t result = cpu->A + value;
  cpu->A = result & 0xFF;

  // Use optimized macro for Z, N, C flags
  UPDATE_ZNC_FLAGS(cpu, cpu->A, result > 0xFF);

  // Handle overflow flag separately (not in macro)
  cpu->flags &= ~FLAG_OVERFLOW;
  cpu->flags |= (((original_A ^ value) & 0x80) == 0 &&
                 ((original_A ^ cpu->A) & 0x80) != 0)
                    ? FLAG_OVERFLOW
                    : 0;
}

static void op_sub(CPU *cpu, uint8_t mode, uint8_t operand) {
  uint8_t value = get_operand_value(cpu, mode, operand);

  uint8_t original_A = cpu->A;
  uint16_t result = cpu->A - value;
  cpu->A = result & 0xFF;

  // Use optimized macro for Z, N, C flags
  UPDATE_ZNC_FLAGS(cpu, cpu->A, result < 0x100);

  // Handle overflow flag separately (not in macro)
  cpu->flags &= ~FLAG_OVERFLOW;
  cpu->flags |= (((original_A ^ value) & 0x80) != 0 &&
                 ((original_A ^ cpu->A) & 0x80) != 0)
                    ? FLAG_OVERFLOW
                    : 0;
}

static void op_and(CPU *cpu, uint8_t mode, uint8_t operand) {
  uint8_t value = get_operand_value(cpu, mode, operand);

  cpu->A &= value;
  UPDATE_ZN_FLAGS(cpu, cpu->A);
}

static void op_xor(CPU *cpu, uint8_t mode, uint8_t operand) {
  uint8_t value = get_operand_value(cpu, mode, operand);

  cpu->A ^= value;
  UPDATE_ZN_FLAGS(cpu, cpu->A);
}

static void op_or(CPU *cpu, uint8_t mode, uint8_t operand) {
  uint8_t value = get_operand_value(cpu, mode, operand);

  cpu->A |= value;
  UPDATE_ZN_FLAGS(cpu, cpu->A);
}

// Optimized branch instruction - most common cases first
static void op_branch(CPU *cpu, uint8_t condition, uint8_t address) {
  // Optimize for most common conditions with direct flag checks
  switch (condition) {
  case COND_AL: // Always - most common, unconditional
    cpu->PC = address;
    break;
  case COND_EQ: // Equal (Z=1) - very common
    if (LIKELY(cpu->flags & FLAG_ZERO))
      cpu->PC = address;
    break;
  case COND_NE: // Not Equal (Z=0) - very common
    if (LIKELY(!(cpu->flags & FLAG_ZERO)))
      cpu->PC = address;
    break;
  case COND_CS: // Carry Set (C=1)
    if (cpu->flags & FLAG_CARRY)
      cpu->PC = address;
    break;
  case COND_CC: // Carry Clear (C=0)
    if (!(cpu->flags & FLAG_CARRY))
      cpu->PC = address;
    break;
  case COND_MI: // Minus/Negative (N=1)
    if (cpu->flags & FLAG_NEGATIVE)
      cpu->PC = address;
    break;
  case COND_PL: // Plus/Positive (N=0)
    if (!(cpu->flags & FLAG_NEGATIVE))
      cpu->PC = address;
    break;
  }
}

static void op_cmp(CPU *cpu, uint8_t mode, uint8_t operand) {
  uint8_t value = get_operand_value(cpu, mode, operand);

  // Perform comparison (A - value) without storing result
  uint8_t original_A = cpu->A;
  uint16_t result = cpu->A - value;
  uint8_t result_byte = result & 0xFF;

  // Use optimized macro for Z, N, C flags
  UPDATE_ZNC_FLAGS(cpu, result_byte, result < 0x100);

  // Handle overflow flag separately (not in macro)
  cpu->flags &= ~FLAG_OVERFLOW;
  cpu->flags |= (((original_A ^ value) & 0x80) != 0 &&
                 ((original_A ^ result_byte) & 0x80) != 0)
                    ? FLAG_OVERFLOW
                    : 0;
}

static void op_cpx(CPU *cpu, uint8_t mode, uint8_t operand) {
  uint8_t value = get_operand_value(cpu, mode, operand);

  // Perform comparison (X - value) without storing result
  uint8_t original_X = cpu->X;
  uint16_t result = cpu->X - value;
  uint8_t result_byte = result & 0xFF;

  // Use optimized macro for Z, N, C flags
  UPDATE_ZNC_FLAGS(cpu, result_byte, result < 0x100);

  // Handle overflow flag separately (not in macro)
  cpu->flags &= ~FLAG_OVERFLOW;
  cpu->flags |= (((original_X ^ value) & 0x80) != 0 &&
                 ((original_X ^ result_byte) & 0x80) != 0)
                    ? FLAG_OVERFLOW
                    : 0;
}

static void op_push(CPU *cpu, uint8_t mode, uint8_t operand) {
  (void)mode; (void)operand;

  if (UNLIKELY(cpu->SP < (STACK_BASE - STACK_SIZE + 1))) {
    cpu->flags |= FLAG_HALTED;  // Stack overflow = halt CPU
    return;
  }

  cpu->memory[cpu->SP] = cpu->A;
  cpu->SP--;
}

static void op_pop(CPU *cpu, uint8_t mode, uint8_t operand) {
  (void)mode; (void)operand;

  if (UNLIKELY(cpu->SP >= STACK_BASE)) {
    cpu->flags |= FLAG_HALTED;  // Stack underflow = halt CPU
    return;
  }

  cpu->SP++;
  cpu->A = cpu->memory[cpu->SP];
  UPDATE_ZN_FLAGS(cpu, cpu->A);
}

static inline void op_halt(CPU *cpu, uint8_t mode, uint8_t operand) {
  (void)mode;
  (void)operand;
  cpu->flags |= FLAG_HALTED;
}

static void op_ror(CPU *cpu, uint8_t mode, uint8_t operand) {
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
    return;
  }

  uint8_t old_carry = (cpu->flags & FLAG_CARRY) ? 1 : 0;
  uint8_t new_carry = value & 1;
  uint8_t result = (uint8_t)((value >> 1) | (old_carry << 7));
  *target = result;

  UPDATE_ZNC_FLAGS(cpu, result, new_carry);
}

static void op_rol(CPU *cpu, uint8_t mode, uint8_t operand) {
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
    return;
  }

  uint8_t old_carry = (cpu->flags & FLAG_CARRY) ? 1 : 0;
  uint8_t new_carry = (value & 0x80) ? 1 : 0;
  uint8_t result = (uint8_t)((value << 1) | old_carry);
  *target = result;

  UPDATE_ZNC_FLAGS(cpu, result, new_carry);
}

static void op_shr(CPU *cpu, uint8_t mode, uint8_t operand) {
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
    return;
  }

  uint8_t new_carry = value & 1;
  uint8_t result = value >> 1;
  *target = result;

  UPDATE_ZNC_FLAGS(cpu, result, new_carry);
}

static void op_shl(CPU *cpu, uint8_t mode, uint8_t operand) {
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
    return;
  }

  uint8_t new_carry = (value & 0x80) ? 1 : 0;
  uint8_t result = (uint8_t)(value << 1);
  *target = result;

  UPDATE_ZNC_FLAGS(cpu, result, new_carry);
}

static void op_inx(CPU *cpu, uint8_t mode, uint8_t operand) {
  (void)mode;
  (void)operand; // INX operates only on X register

  cpu->X++;
  UPDATE_ZN_FLAGS(cpu, cpu->X);
}

static void op_dex(CPU *cpu, uint8_t mode, uint8_t operand) {
  (void)mode;
  (void)operand; // DEX operates only on X register

  cpu->X--;
  UPDATE_ZN_FLAGS(cpu, cpu->X);
}

// Optimized dispatch table with better cache layout
static const opcode_handler handlers[OPCODE_COUNT] = {
    [OPCODE_NOP] = op_nop,   [OPCODE_LDA] = op_lda,  [OPCODE_LDX] = op_ldx,
    [OPCODE_ADD] = op_add,   [OPCODE_SUB] = op_sub,  [OPCODE_XOR] = op_xor,
    [OPCODE_STA] = op_sta,   [OPCODE_STX] = op_stx,  [OPCODE_AND] = op_and,
    [OPCODE_OR] = op_or,     [OPCODE_B] = op_branch, [OPCODE_POP] = op_pop,
    [OPCODE_PUSH] = op_push, [OPCODE_CMP] = op_cmp,  [OPCODE_CPX] = op_cpx,
    [OPCODE_HALT] = op_halt, [OPCODE_ROR] = op_ror,  [OPCODE_ROL] = op_rol,
    [OPCODE_SHR] = op_shr,   [OPCODE_SHL] = op_shl,  [OPCODE_INX] = op_inx,
    [OPCODE_DEX] = op_dex,
};

_Static_assert(OPCODE_COUNT == (sizeof handlers / sizeof handlers[0]),
               "opcode count mismatch");

// CPU initialization with optimized memset
static inline void initCPU(CPU *cpu) {
  __builtin_memset(cpu, 0, sizeof(CPU));
  cpu->SP = STACK_BASE;
}

// ULTRA-OPTIMIZED cpu_step - void handlers, zero return value overhead
static inline int cpu_step(CPU *cpu) {
  // Fetch opcode with optimized error checking
  uint8_t opcode = cpu->memory[cpu->PC++];

  if (UNLIKELY(opcode >= OPCODE_COUNT || !handlers[opcode])) {
    cpu->flags |= FLAG_HALTED;
    return CPU_HALTED;
  }

  // Fetch mode and operand
  uint8_t mode = cpu->memory[cpu->PC++];
  uint8_t operand = cpu->memory[cpu->PC++];

  // Special case: Branch instruction uses condition, not mode
  if (opcode == OPCODE_B) {
    // Don't validate mode for branch - it's actually a condition
  } else {
    // Validate mode for all other instructions
    if (UNLIKELY(mode >= MODE_COUNT)) {
      cpu->flags |= FLAG_HALTED;
      return CPU_HALTED;
    }
  }
  // Execute instruction - no return value overhead!
  handlers[opcode](cpu, mode, operand);
  return (cpu->flags & FLAG_HALTED) ? CPU_HALTED : CPU_OK;
}

// cpu_step_packed - void handlers, zero return value overhead
static inline int cpu_step_packed(CPU *cpu) {
  /* Fetch packed byte */
  uint8_t packed = cpu->memory[cpu->PC++];

  uint8_t opcode = UNPACK_OPCODE(packed);
  uint8_t mode = UNPACK_MODE(packed);

  if (UNLIKELY(opcode >= OPCODE_COUNT || !handlers[opcode])) {
    cpu->flags |= FLAG_HALTED;
    return CPU_HALTED;
  }

  // Special case: Branch instruction uses condition, not mode
  if (opcode == OPCODE_B) {
    // Don't validate mode for branch - it's actually a condition
  } else {
    // Validate mode for all other instructions
    if (UNLIKELY(mode >= MODE_COUNT)) {
      cpu->flags |= FLAG_HALTED;
      return CPU_HALTED;
    }
  }

  /* Fetch operand (still one byte) */
  uint8_t operand = cpu->memory[cpu->PC++];

  // Execute instruction - no return value overhead!
  handlers[opcode](cpu, mode, operand);
  return (cpu->flags & FLAG_HALTED) ? CPU_HALTED : CPU_OK;
}

/* cpu_run that accepts a step-function pointer.
   We capture the instruction start PC (prev_pc) so the reporting works
   independently of the exact decoding/length policy of the step function. */
static inline void cpu_run(CPU *cpu, cpu_step_fn step) {
  int result;
  while (1) {
    uint8_t prev_pc = cpu->PC; /* instruction start (for reporting) */
    result = step(cpu);

    if (UNLIKELY(result == CPU_HALTED)) {
      printf("CPU HALTED at PC=0x%02X\n", prev_pc);
      break;
    } else if (UNLIKELY(result == CPU_HALTED)) {
      printf("CPU ERROR at PC=0x%02X\n", prev_pc);
      break;
    }
  }
}

#endif // CPU_H
