#include "unity/unity.h"
#include "../cpu.h"

void PUSH_test(void) {
    CPU cpu;
    initCPU(&cpu);
    cpu.A = 0xAA;
    cpu.PC = 0;
    // PUSH ; A = $AA (170), memory[$FF] = $AA (170), SP = $FE (254)
    cpu.memory[0] = OPCODE_PUSH;
    cpu.memory[1] = MODE_IMMEDIAT;
    cpu.memory[2] = 0x00;
    TEST_ASSERT_EQUAL_INT(CPU_OK, cpu_step(&cpu));
    TEST_ASSERT_EQUAL_UINT8(0xAA, cpu.memory[STACK_BASE]);
    TEST_ASSERT_EQUAL_UINT8(STACK_BASE - 1, cpu.SP);
}
