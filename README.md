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
    OPCODE_HALT    = 0x0F, // Halt
    OPCODE_ROR     = 0x10, // Rotate Right
    OPCODE_ROL     = 0x11, // Rotate Left
    OPCODE_SHR     = 0x12, // Shift Right
    OPCODE_SHL     = 0x13, // Shift Left
    OPCODE_INX     = 0x14, // Increment X
    OPCODE_DEX     = 0x15  // Decrement X
};

// Addressing modes
enum {
    MODE_IMMEDIAT  = 0x00,   // Immediate:  #value
    MODE_ABSOLUTE  = 0x01,   // Absolute:   address
    MODE_ABSOLUTE_X  = 0x02, // Indexed:    address,X
    MODE_INDIRECT  = 0x03,   // Indirect:   [$address]
    MODE_INDIRECT_X = 0x04,  // Indirect Indexed: [$address,X]
    MODE_REGISTER = 0x05     // REGISTER A
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
```

+-----------------------+-----------------------+--------------------------------------------------------------+
| ADDRESSING MODE       | SYNTAX                | DESCRIPTION & EXAMPLES                                       |
+-----------------------+-----------------------+--------------------------------------------------------------+
| MODE_IMMEDIAT (0x00)  | LDA #$42              | Load immediate value into A                                  |
|                       | ❌ STA #$42 (invalid) | • Value is directly in instruction                           |
|                       |                       | • Example: A = $42                                           |
+-----------------------+-----------------------+--------------------------------------------------------------+
| MODE_ABSOLUTE (0x01)  | LDA $30               | Load from absolute memory address                            |
|                       | STA $30               | • Direct memory access                                       |
|                       |                       | • Example: A = memory[$30] or memory[$30] = A                |
+-----------------------+-----------------------+--------------------------------------------------------------+
| MODE_ABSOLUTE_X       | LDA $30,X             | Load from indexed memory address                             |
| (0x02)                | STA $30,X             | • Address = base + X register                                |
|                       |                       | • Example: A = memory[$30 + X] or memory[$30 + X] = A        |
+-----------------------+-----------------------+--------------------------------------------------------------+
| MODE_INDIRECT (0x03)  | LDA ($40)             | Load using indirect addressing                               |
|                       | STA ($40)             | • Two-step: get address from memory, then access that address|
|                       |                       | • Example: A = memory[memory[$40]] or memory[memory[$40]] = A|
+-----------------------+-----------------------+--------------------------------------------------------------+
| MODE_INDIRECT_X       | LDA ($60,X)           | Load using indirect indexed addressing                       |
| (0x04)                | STA ($60,X)           | • Address = memory[base + X], then access that address       |
|                       |                       | • Example: A = memory[memory[$60 + X]]                       |
+-----------------------+-----------------------+--------------------------------------------------------------+
| MODE_REGISTER (0x05)  | ROR A                 | Operate directly on accumulator register                     |
|                       | ❌ STA A (invalid)     | • Used for shift/rotate operations only                      |
|                       |                       | • Example: Rotate bits in A register                         |
+-----------------------+-----------------------+--------------------------------------------------------------+
