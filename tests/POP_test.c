#include "unity/unity.h"
#include "../cpu.h"

void POP_test(void) {
    CPU cpu;
    initCPU(&cpu);
    cpu.SP = STACK_BASE - 1;
    cpu.memory[cpu.SP + 1] = 0x55;
    cpu.A = 0x00;
    cpu.PC = 0;
    // POP ; SP = $FE (254), memory[$FF] = $55 (85), A = $55 (85), SP = $FF (255)
    cpu.memory[0] = OPCODE_POP;
    cpu.memory[1] = MODE_IMMEDIAT;
    cpu.memory[2] = 0x00;
    TEST_ASSERT_EQUAL_INT(CPU_OK, cpu_step(&cpu));
    TEST_ASSERT_EQUAL_UINT8(0x55, cpu.A);
    TEST_ASSERT_EQUAL_UINT8(STACK_BASE, cpu.SP);
}
