#include "unity/unity.h"
#include "../cpu.h"

void NOP_test(void) {
    CPU cpu;
    initCPU(&cpu);
    // NOP #$00 ; No operation, PC = $00 (0) + 3 => $03 (3), flags unchanged
    cpu.PC = 0;
    cpu.memory[0] = OPCODE_NOP;
    uint8_t oldA = cpu.A, oldX = cpu.X, oldPC = cpu.PC;
    cpu.flags = FLAG_CARRY | FLAG_ZERO | FLAG_NEGATIVE | FLAG_OVERFLOW ; // Set all flags
    uint8_t oldFlags = cpu.flags;
    TEST_ASSERT_EQUAL_INT(CPU_OK, cpu_step(&cpu));
    TEST_ASSERT_EQUAL_UINT8(oldA, cpu.A);
    TEST_ASSERT_EQUAL_UINT8(oldX, cpu.X);
    TEST_ASSERT_EQUAL_UINT8(oldPC + 3, cpu.PC);
    TEST_ASSERT_EQUAL_UINT8(oldFlags, cpu.flags);
}
