# VM CPU 8-bit

- A simple virtual CPU 8-bit, a very good starting point.
- Clear segregation of components, easy to understand and extend up to 256 for both opcodes and modes.
- Unit tests included.
- Simple Makefile for Linux and macOS.

## Architecture et approche C
- **Registers:** Accumulator (A), index register (X), program counter (PC), stack pointer (SP), flags register (SR).
- **Memory:** 256 bytes (No memory allocation for simplicity).
- **Instructions:** Every opcodes are associated to a function handler.
- **CPU structure:** All in a C structure CPU:

```c
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
```

| ADDRESSING MODE       | SYNTAX                | DESCRIPTION & EXAMPLES                                       |
|-----------------------|-----------------------|--------------------------------------------------------------|
| MODE_IMMEDIAT (0x00)  | LDA #$42              | Load immediate value into A                                  |
|                       | ❌ STA #$42 (invalid) | • Value is directly in instruction                           |
|                       |                       | • Example: A = $42                                           |
|-----------------------|-----------------------|--------------------------------------------------------------|
| MODE_ABSOLUTE (0x01)  | LDA $30               | Load from absolute memory address                            |
|                       | STA $30               | • Direct memory access                                       |
|                       |                       | • Example: A = memory[$30] or memory[$30] = A                |
|-----------------------|-----------------------|--------------------------------------------------------------|
| MODE_ABSOLUTE_X       | LDA $30,X             | Load from indexed memory address                             |
| (0x02)                | STA $30,X             | • Address = base + X register                                |
|                       |                       | • Example: A = memory[$30 + X] or memory[$30 + X] = A        |
|-----------------------|-----------------------|--------------------------------------------------------------|
| MODE_INDIRECT (0x03)  | LDA ($40)             | Load using indirect addressing                               |
|                       | STA ($40)             | • Two-step: get address from memory, then access that address|
|                       |                       | • Example: A = memory[memory[$40]] or memory[memory[$40]] = A|
|-----------------------|-----------------------|--------------------------------------------------------------|
| MODE_INDIRECT_X       | LDA ($60,X)           | Load using indirect indexed addressing                       |
| (0x04)                | STA ($60,X)           | • Address = memory[base + X], then access that address       |
|                       |                       | • Example: A = memory[memory[$60 + X]]                       |
|-----------------------|-----------------------|--------------------------------------------------------------|
| MODE_REGISTER (0x05)  | ROR A                 | Operate directly on accumulator register                     |
|                       | ❌ STA A (invalid)     | • Used for shift/rotate operations only                      |
|                       |                       | • Example: Rotate bits in A register                         |
