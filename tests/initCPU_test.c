#include "unity/unity.h"
#include "../cpu.h"

void initCPU_test(void) {
    CPU cpu;
    initCPU(&cpu);

    for (int i=0;i<256;i++)
        TEST_ASSERT_EQUAL_UINT8(0x00, cpu.memory[i]);

    // After initCPU: A = $00 (0), X = $00 (0), PC = $00 (0), SP = $FF (255), flags = $00 (0)
    TEST_ASSERT_EQUAL_UINT8(0, cpu.A);
    TEST_ASSERT_EQUAL_UINT8(0, cpu.X);
    TEST_ASSERT_EQUAL_UINT8(0, cpu.PC);
    TEST_ASSERT_EQUAL_UINT8(0xFF, cpu.SP);
    TEST_ASSERT_EQUAL_UINT8(0, cpu.flags);

    TEST_ASSERT_FALSE(cpu.flags & FLAG_CARRY);
    TEST_ASSERT_FALSE(cpu.flags & FLAG_ZERO);
    TEST_ASSERT_FALSE(cpu.flags & FLAG_NEGATIVE);
    TEST_ASSERT_FALSE(cpu.flags & FLAG_OVERFLOW);
    TEST_ASSERT_FALSE(cpu.flags & FLAG_HALTED);

    // Test that setting each flag is detected
    cpu.flags = FLAG_CARRY;
    TEST_ASSERT_TRUE(cpu.flags & FLAG_CARRY);
    cpu.flags = FLAG_ZERO;
    TEST_ASSERT_TRUE(cpu.flags & FLAG_ZERO);
    cpu.flags = FLAG_NEGATIVE;
    TEST_ASSERT_TRUE(cpu.flags & FLAG_NEGATIVE);
    cpu.flags = FLAG_OVERFLOW;
    TEST_ASSERT_TRUE(cpu.flags & FLAG_OVERFLOW);
    cpu.flags = FLAG_HALTED;
    TEST_ASSERT_TRUE(cpu.flags & FLAG_HALTED);
}
