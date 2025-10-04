#include "unity/unity.h"
#include "../cpu.h"

void initCPU_test(void) {
    CPU cpu;
    initCPU(&cpu);

    for (int i=0;i<256;i++)
        TEST_ASSERT_EQUAL_UINT8(0x00, cpu.memory[i]);

    TEST_ASSERT_EQUAL_UINT8(0x00, cpu.PC);
    TEST_ASSERT_EQUAL_UINT8(0xFF, cpu.SP);
}
