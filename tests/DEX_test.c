#include "unity/unity.h"
#include "../cpu.h"

void DEX_test(void) {
    CPU cpu;
    
    // Test 1: DEX - Decrement X from 1 to 0 (result is zero)
    initCPU(&cpu);
    cpu.X = 0x01;
    cpu.PC = 0;
    cpu.memory[0] = OPCODE_DEX;
    cpu.memory[1] = 0x00;  // Mode unused for DEX
    cpu.memory[2] = 0x00;  // Operand unused for DEX
    TEST_ASSERT_EQUAL_INT(CPU_OK, cpu_step(&cpu));
    TEST_ASSERT_EQUAL_UINT8(0x00, cpu.X);
    TEST_ASSERT_TRUE(cpu.flags & FLAG_ZERO);
    TEST_ASSERT_FALSE(cpu.flags & FLAG_NEGATIVE);

    // Test 2: DEX - Decrement X from 43 to 42
    initCPU(&cpu);
    cpu.X = 0x43;
    cpu.PC = 0;
    cpu.memory[0] = OPCODE_DEX;
    cpu.memory[1] = 0x00;
    cpu.memory[2] = 0x00;
    TEST_ASSERT_EQUAL_INT(CPU_OK, cpu_step(&cpu));
    TEST_ASSERT_EQUAL_UINT8(0x42, cpu.X);
    TEST_ASSERT_FALSE(cpu.flags & FLAG_ZERO);
    TEST_ASSERT_FALSE(cpu.flags & FLAG_NEGATIVE);

    // Test 3: DEX - Decrement X from 0x00 to 0xFF (wraparound)
    initCPU(&cpu);
    cpu.X = 0x00;
    cpu.PC = 0;
    cpu.memory[0] = OPCODE_DEX;
    cpu.memory[1] = 0x00;
    cpu.memory[2] = 0x00;
    TEST_ASSERT_EQUAL_INT(CPU_OK, cpu_step(&cpu));
    TEST_ASSERT_EQUAL_UINT8(0xFF, cpu.X);
    TEST_ASSERT_FALSE(cpu.flags & FLAG_ZERO);
    TEST_ASSERT_TRUE(cpu.flags & FLAG_NEGATIVE);

    // Test 4: DEX - Decrement X from 0x80 to 0x7F (negative to positive)
    initCPU(&cpu);
    cpu.X = 0x80;  // 128 (negative in signed)
    cpu.PC = 0;
    cpu.memory[0] = OPCODE_DEX;
    cpu.memory[1] = 0x00;
    cpu.memory[2] = 0x00;
    TEST_ASSERT_EQUAL_INT(CPU_OK, cpu_step(&cpu));
    TEST_ASSERT_EQUAL_UINT8(0x7F, cpu.X);  // 127 (positive in signed)
    TEST_ASSERT_FALSE(cpu.flags & FLAG_ZERO);
    TEST_ASSERT_FALSE(cpu.flags & FLAG_NEGATIVE);

    // Test 5: DEX - Decrement X from 0xFF to 0xFE (stays negative)
    initCPU(&cpu);
    cpu.X = 0xFF;
    cpu.PC = 0;
    cpu.memory[0] = OPCODE_DEX;
    cpu.memory[1] = 0x00;
    cpu.memory[2] = 0x00;
    TEST_ASSERT_EQUAL_INT(CPU_OK, cpu_step(&cpu));
    TEST_ASSERT_EQUAL_UINT8(0xFE, cpu.X);
    TEST_ASSERT_FALSE(cpu.flags & FLAG_ZERO);
    TEST_ASSERT_TRUE(cpu.flags & FLAG_NEGATIVE);

    // Test 6: DEX - Verify A register is not affected
    initCPU(&cpu);
    cpu.A = 0x55;
    cpu.X = 0x10;
    cpu.PC = 0;
    cpu.memory[0] = OPCODE_DEX;
    cpu.memory[1] = 0x00;
    cpu.memory[2] = 0x00;
    TEST_ASSERT_EQUAL_INT(CPU_OK, cpu_step(&cpu));
    TEST_ASSERT_EQUAL_UINT8(0x55, cpu.A);  // A should be unchanged
    TEST_ASSERT_EQUAL_UINT8(0x0F, cpu.X);

    // Test 7: DEX - Chain multiple decrements
    initCPU(&cpu);
    cpu.X = 0x15;  // Start at 21
    
    // First decrement: 0x15 -> 0x14
    cpu.PC = 0;
    cpu.memory[0] = OPCODE_DEX;
    cpu.memory[1] = 0x00;
    cpu.memory[2] = 0x00;
    TEST_ASSERT_EQUAL_INT(CPU_OK, cpu_step(&cpu));
    TEST_ASSERT_EQUAL_UINT8(0x14, cpu.X);
    TEST_ASSERT_FALSE(cpu.flags & FLAG_ZERO);
    TEST_ASSERT_FALSE(cpu.flags & FLAG_NEGATIVE);
    
    // Second decrement: 0x14 -> 0x13
    cpu.PC = 3;
    cpu.memory[3] = OPCODE_DEX;
    cpu.memory[4] = 0x00;
    cpu.memory[5] = 0x00;
    TEST_ASSERT_EQUAL_INT(CPU_OK, cpu_step(&cpu));
    TEST_ASSERT_EQUAL_UINT8(0x13, cpu.X);
    TEST_ASSERT_FALSE(cpu.flags & FLAG_ZERO);
    TEST_ASSERT_FALSE(cpu.flags & FLAG_NEGATIVE);
    
    // Third decrement: 0x13 -> 0x12
    cpu.PC = 6;
    cpu.memory[6] = OPCODE_DEX;
    cpu.memory[7] = 0x00;
    cpu.memory[8] = 0x00;
    TEST_ASSERT_EQUAL_INT(CPU_OK, cpu_step(&cpu));
    TEST_ASSERT_EQUAL_UINT8(0x12, cpu.X);
    TEST_ASSERT_FALSE(cpu.flags & FLAG_ZERO);
    TEST_ASSERT_FALSE(cpu.flags & FLAG_NEGATIVE);

    // Test 8: DEX - Typical countdown loop usage
    initCPU(&cpu);
    cpu.X = 0x05;  // Start countdown from 5
    
    // Simulate a countdown loop
    for (int i = 5; i > 0; i--) {
        cpu.PC = (uint8_t)((5 - i) * 3);
        cpu.memory[cpu.PC] = OPCODE_DEX;
        cpu.memory[cpu.PC + 1] = 0x00;
        cpu.memory[cpu.PC + 2] = 0x00;
        TEST_ASSERT_EQUAL_INT(CPU_OK, cpu_step(&cpu));
        TEST_ASSERT_EQUAL_UINT8(i - 1, cpu.X);
        if (i == 1) {
            TEST_ASSERT_TRUE(cpu.flags & FLAG_ZERO);  // Should be zero when we reach 0
        } else {
            TEST_ASSERT_FALSE(cpu.flags & FLAG_ZERO);
        }
    }

    // Test 9: DEX - Verify flags behavior with previous flags set
    initCPU(&cpu);
    cpu.X = 0x80;
    cpu.flags = FLAG_CARRY | FLAG_OVERFLOW;  // Set some other flags
    cpu.PC = 0;
    cpu.memory[0] = OPCODE_DEX;
    cpu.memory[1] = 0x00;
    cpu.memory[2] = 0x00;
    TEST_ASSERT_EQUAL_INT(CPU_OK, cpu_step(&cpu));
    TEST_ASSERT_EQUAL_UINT8(0x7F, cpu.X);
    // DEX should only affect ZERO and NEGATIVE flags
    TEST_ASSERT_TRUE(cpu.flags & FLAG_CARRY);    // Should remain set
    TEST_ASSERT_TRUE(cpu.flags & FLAG_OVERFLOW); // Should remain set
    TEST_ASSERT_FALSE(cpu.flags & FLAG_ZERO);
    TEST_ASSERT_FALSE(cpu.flags & FLAG_NEGATIVE);

    // Test 10: DEX - From 0x81 to 0x80 (negative to negative)
    initCPU(&cpu);
    cpu.X = 0x81;
    cpu.PC = 0;
    cpu.memory[0] = OPCODE_DEX;
    cpu.memory[1] = 0x00;
    cpu.memory[2] = 0x00;
    TEST_ASSERT_EQUAL_INT(CPU_OK, cpu_step(&cpu));
    TEST_ASSERT_EQUAL_UINT8(0x80, cpu.X);
    TEST_ASSERT_FALSE(cpu.flags & FLAG_ZERO);
    TEST_ASSERT_TRUE(cpu.flags & FLAG_NEGATIVE);

    // Test 11: DEX - Edge case around middle values
    initCPU(&cpu);
    cpu.X = 0x41;  // 65
    cpu.PC = 0;
    cpu.memory[0] = OPCODE_DEX;
    cpu.memory[1] = 0x00;
    cpu.memory[2] = 0x00;
    TEST_ASSERT_EQUAL_INT(CPU_OK, cpu_step(&cpu));
    TEST_ASSERT_EQUAL_UINT8(0x40, cpu.X);  // 64
    TEST_ASSERT_FALSE(cpu.flags & FLAG_ZERO);
    TEST_ASSERT_FALSE(cpu.flags & FLAG_NEGATIVE);

    // Test 12: DEX - Verify PC advancement
    initCPU(&cpu);
    cpu.X = 0x50;
    cpu.PC = 0x20;  // Start at different PC
    cpu.memory[0x20] = OPCODE_DEX;
    cpu.memory[0x21] = 0x00;
    cpu.memory[0x22] = 0x00;
    TEST_ASSERT_EQUAL_INT(CPU_OK, cpu_step(&cpu));
    TEST_ASSERT_EQUAL_UINT8(0x4F, cpu.X);
    TEST_ASSERT_EQUAL_UINT8(0x23, cpu.PC);  // PC should advance by 3

    // Test 13: DEX - Multiple wraparounds
    initCPU(&cpu);
    cpu.X = 0x02;
    
    // First: 2 -> 1
    cpu.PC = 0;
    cpu.memory[0] = OPCODE_DEX;
    cpu.memory[1] = 0x00;
    cpu.memory[2] = 0x00;
    TEST_ASSERT_EQUAL_INT(CPU_OK, cpu_step(&cpu));
    TEST_ASSERT_EQUAL_UINT8(0x01, cpu.X);
    TEST_ASSERT_FALSE(cpu.flags & FLAG_ZERO);
    
    // Second: 1 -> 0
    cpu.PC = 3;
    cpu.memory[3] = OPCODE_DEX;
    cpu.memory[4] = 0x00;
    cpu.memory[5] = 0x00;
    TEST_ASSERT_EQUAL_INT(CPU_OK, cpu_step(&cpu));
    TEST_ASSERT_EQUAL_UINT8(0x00, cpu.X);
    TEST_ASSERT_TRUE(cpu.flags & FLAG_ZERO);
    
    // Third: 0 -> 255 (wraparound)
    cpu.PC = 6;
    cpu.memory[6] = OPCODE_DEX;
    cpu.memory[7] = 0x00;
    cpu.memory[8] = 0x00;
    TEST_ASSERT_EQUAL_INT(CPU_OK, cpu_step(&cpu));
    TEST_ASSERT_EQUAL_UINT8(0xFF, cpu.X);
    TEST_ASSERT_FALSE(cpu.flags & FLAG_ZERO);
    TEST_ASSERT_TRUE(cpu.flags & FLAG_NEGATIVE);
}
