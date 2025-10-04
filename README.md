# VM avec CPU 8 bits

Machine virtuelle simple avec un CPU 8 bits.

```c
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
OPCODE_B       = 0x0A, // Branch (conditional)
OPCODE_POP     = 0x0B, // Pop from Stack
OPCODE_PUSH    = 0x0C, // Push to Stack
OPCODE_HALT    = 0x0F  // Halt
```

```c
MODE_IMM = 0x00,  // Immédiat:  #value
MODE_ABS = 0x01,  // Absolu:    address
MODE_IDX = 0x02,  // Indexé:    address,X
```

## Architecture et approche C
- **Registres:** Accumulateur (A), registre d’index (X), compteur de programme (PC), pointeur de pile (SP), flags de statuts (SR).
- **Mémoire:** 256 octets, adressées via un tableau.
- **Instructions:** Chaque opcode est associé à une fonction handler en C via un pointeur de fonction.
- **Structure CPU:** Tout est regroupé dans une structure C :

```c
typedef struct {
    uint8_t A;      // Accumulateur
    uint8_t X;      // Registre d’index
    uint8_t PC;     // Compteur de programme
    uint8_t SP;     // Pointeur de pile
    uint8_t flags;  // Flags de statut
    uint8_t memory[MAX_MEMORY_SIZE]; // Mémoire
} CPU __attribute__((aligned(64)));
```
