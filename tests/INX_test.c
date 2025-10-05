#include "unity/unity.h"
#include "../cpu.h"

void INX_test(void) {
    CPU cpu;
    
    // Test 1: INX - Increment X from 0 to 1
    initCPU(&cpu);
    cpu.X = 0x00;
    cpu.PC = 0;
    cpu.memory[0] = OPCODE_INX;
    cpu.memory[1] = 0x00;  // Mode unused for INX
    cpu.memory[2] = 0x00;  // Operand unused for INX
    TEST_ASSERT_EQUAL_INT(CPU_OK, cpu_step(&cpu));
    TEST_ASSERT_EQUAL_UINT8(0x01, cpu.X);
    TEST_ASSERT_FALSE(cpu.flags & FLAG_ZERO);
    TEST_ASSERT_FALSE(cpu.flags & FLAG_NEGATIVE);

    // Test 2: INX - Increment X from 42 to 43
    initCPU(&cpu);
    cpu.X = 0x42;
    cpu.PC = 0;
    cpu.memory[0] = OPCODE_INX;
    cpu.memory[1] = 0x00;
    cpu.memory[2] = 0x00;
    TEST_ASSERT_EQUAL_INT(CPU_OK, cpu_step(&cpu));
    TEST_ASSERT_EQUAL_UINT8(0x43, cpu.X);
    TEST_ASSERT_FALSE(cpu.flags & FLAG_ZERO);
    TEST_ASSERT_FALSE(cpu.flags & FLAG_NEGATIVE);

    // Test 3: INX - Increment X from 0xFF to 0x00 (wraparound)
    initCPU(&cpu);
    cpu.X = 0xFF;
    cpu.PC = 0;
    cpu.memory[0] = OPCODE_INX;
    cpu.memory[1] = 0x00;
    cpu.memory[2] = 0x00;
    TEST_ASSERT_EQUAL_INT(CPU_OK, cpu_step(&cpu));
    TEST_ASSERT_EQUAL_UINT8(0x00, cpu.X);
    TEST_ASSERT_TRUE(cpu.flags & FLAG_ZERO);
    TEST_ASSERT_FALSE(cpu.flags & FLAG_NEGATIVE);

    // Test 4: INX - Increment X from 0x7F to 0x80 (positive to negative)
    initCPU(&cpu);
    cpu.X = 0x7F;  // 127 (maximum positive in signed)
    cpu.PC = 0;
    cpu.memory[0] = OPCODE_INX;
    cpu.memory[1] = 0x00;
    cpu.memory[2] = 0x00;
    TEST_ASSERT_EQUAL_INT(CPU_OK, cpu_step(&cpu));
    TEST_ASSERT_EQUAL_UINT8(0x80, cpu.X);  // 128 (negative in signed)
    TEST_ASSERT_FALSE(cpu.flags & FLAG_ZERO);
    TEST_ASSERT_TRUE(cpu.flags & FLAG_NEGATIVE);

    // Test 5: INX - Increment X from 0xFE to 0xFF (stays negative)
    initCPU(&cpu);
    cpu.X = 0xFE;
    cpu.PC = 0;
    cpu.memory[0] = OPCODE_INX;
    cpu.memory[1] = 0x00;
    cpu.memory[2] = 0x00;
    TEST_ASSERT_EQUAL_INT(CPU_OK, cpu_step(&cpu));
    TEST_ASSERT_EQUAL_UINT8(0xFF, cpu.X);
    TEST_ASSERT_FALSE(cpu.flags & FLAG_ZERO);
    TEST_ASSERT_TRUE(cpu.flags & FLAG_NEGATIVE);

    // Test 6: INX - Verify A register is not affected
    initCPU(&cpu);
    cpu.A = 0x55;
    cpu.X = 0x10;
    cpu.PC = 0;
    cpu.memory[0] = OPCODE_INX;
    cpu.memory[1] = 0x00;
    cpu.memory[2] = 0x00;
    TEST_ASSERT_EQUAL_INT(CPU_OK, cpu_step(&cpu));
    TEST_ASSERT_EQUAL_UINT8(0x55, cpu.A);  // A should be unchanged
    TEST_ASSERT_EQUAL_UINT8(0x11, cpu.X);

    // Test 7: INX - Chain multiple increments
    initCPU(&cpu);
    cpu.X = 0x10;
    
    // First increment: 0x10 -> 0x11
    cpu.PC = 0;
    cpu.memory[0] = OPCODE_INX;
    cpu.memory[1] = 0x00;
    cpu.memory[2] = 0x00;
    TEST_ASSERT_EQUAL_INT(CPU_OK, cpu_step(&cpu));
    TEST_ASSERT_EQUAL_UINT8(0x11, cpu.X);
    TEST_ASSERT_FALSE(cpu.flags & FLAG_ZERO);
    TEST_ASSERT_FALSE(cpu.flags & FLAG_NEGATIVE);
    
    // Second increment: 0x11 -> 0x12
    cpu.PC = 3;
    cpu.memory[3] = OPCODE_INX;
    cpu.memory[4] = 0x00;
    cpu.memory[5] = 0x00;
    TEST_ASSERT_EQUAL_INT(CPU_OK, cpu_step(&cpu));
    TEST_ASSERT_EQUAL_UINT8(0x12, cpu.X);
    TEST_ASSERT_FALSE(cpu.flags & FLAG_ZERO);
    TEST_ASSERT_FALSE(cpu.flags & FLAG_NEGATIVE);
    
    // Third increment: 0x12 -> 0x13
    cpu.PC = 6;
    cpu.memory[6] = OPCODE_INX;
    cpu.memory[7] = 0x00;
    cpu.memory[8] = 0x00;
    TEST_ASSERT_EQUAL_INT(CPU_OK, cpu_step(&cpu));
    TEST_ASSERT_EQUAL_UINT8(0x13, cpu.X);
    TEST_ASSERT_FALSE(cpu.flags & FLAG_ZERO);
    TEST_ASSERT_FALSE(cpu.flags & FLAG_NEGATIVE);

    // Test 8: INX - Typical loop counter usage
    initCPU(&cpu);
    cpu.X = 0x00;
    
    // Simulate a simple counting loop
    for (int i = 0; i < 10; i++) {
        cpu.PC = (uint8_t)(i * 3);
        cpu.memory[cpu.PC] = OPCODE_INX;
        cpu.memory[cpu.PC + 1] = 0x00;
        cpu.memory[cpu.PC + 2] = 0x00;
        TEST_ASSERT_EQUAL_INT(CPU_OK, cpu_step(&cpu));
        TEST_ASSERT_EQUAL_UINT8(i + 1, cpu.X);
    }

    // Test 9: INX - Verify flags behavior with previous flags set
    initCPU(&cpu);
    cpu.X = 0x7F;
    cpu.flags = FLAG_CARRY | FLAG_OVERFLOW;  // Set some other flags
    cpu.PC = 0;
    cpu.memory[0] = OPCODE_INX;
    cpu.memory[1] = 0x00;
    cpu.memory[2] = 0x00;
    TEST_ASSERT_EQUAL_INT(CPU_OK, cpu_step(&cpu));
    TEST_ASSERT_EQUAL_UINT8(0x80, cpu.X);
    // INX should only affect ZERO and NEGATIVE flags
    TEST_ASSERT_TRUE(cpu.flags & FLAG_CARRY);    // Should remain set
    TEST_ASSERT_TRUE(cpu.flags & FLAG_OVERFLOW); // Should remain set
    TEST_ASSERT_FALSE(cpu.flags & FLAG_ZERO);
    TEST_ASSERT_TRUE(cpu.flags & FLAG_NEGATIVE);

    // Test 10: INX - From 0x80 to 0x81 (negative to negative)
    initCPU(&cpu);
    cpu.X = 0x80;
    cpu.PC = 0;
    cpu.memory[0] = OPCODE_INX;
    cpu.memory[1] = 0x00;
    cpu.memory[2] = 0x00;
    TEST_ASSERT_EQUAL_INT(CPU_OK, cpu_step(&cpu));
    TEST_ASSERT_EQUAL_UINT8(0x81, cpu.X);
    TEST_ASSERT_FALSE(cpu.flags & FLAG_ZERO);
    TEST_ASSERT_TRUE(cpu.flags & FLAG_NEGATIVE);

    // Test 11: INX - Edge case around middle values
    initCPU(&cpu);
    cpu.X = 0x3F;  // 63
    cpu.PC = 0;
    cpu.memory[0] = OPCODE_INX;
    cpu.memory[1] = 0x00;
    cpu.memory[2] = 0x00;
    TEST_ASSERT_EQUAL_INT(CPU_OK, cpu_step(&cpu));
    TEST_ASSERT_EQUAL_UINT8(0x40, cpu.X);  // 64
    TEST_ASSERT_FALSE(cpu.flags & FLAG_ZERO);
    TEST_ASSERT_FALSE(cpu.flags & FLAG_NEGATIVE);

    // Test 12: INX - Verify PC advancement
    initCPU(&cpu);
    cpu.X = 0x50;
    cpu.PC = 0x20;  // Start at different PC
    cpu.memory[0x20] = OPCODE_INX;
    cpu.memory[0x21] = 0x00;
    cpu.memory[0x22] = 0x00;
    TEST_ASSERT_EQUAL_INT(CPU_OK, cpu_step(&cpu));
    TEST_ASSERT_EQUAL_UINT8(0x51, cpu.X);
    TEST_ASSERT_EQUAL_UINT8(0x23, cpu.PC);  // PC should advance by 3
}
