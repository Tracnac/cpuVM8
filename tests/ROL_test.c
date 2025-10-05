#include "unity/unity.h"
#include "../cpu.h"

void ROL_test(void) {
    CPU cpu;
    
    // Test 1: ROL MODE_REGISTER - Rotate accumulator left without carry
    initCPU(&cpu);
    cpu.A = 0x42;  // 01000010
    cpu.flags = 0;  // Clear carry
    cpu.PC = 0;
    cpu.memory[0] = OPCODE_ROL;
    cpu.memory[1] = MODE_REGISTER;
    cpu.memory[2] = 0x00;  // Operand unused
    TEST_ASSERT_EQUAL_INT(CPU_OK, cpu_step(&cpu));
    TEST_ASSERT_EQUAL_UINT8(0x84, cpu.A);  // 10000100
    TEST_ASSERT_FALSE(cpu.flags & FLAG_CARRY);  // Bit 7 was 0
    TEST_ASSERT_FALSE(cpu.flags & FLAG_ZERO);
    TEST_ASSERT_TRUE(cpu.flags & FLAG_NEGATIVE);

    // Test 2: ROL MODE_REGISTER - Rotate accumulator left with carry in
    initCPU(&cpu);
    cpu.A = 0x40;  // 01000000
    cpu.flags = FLAG_CARRY;  // Set carry
    cpu.PC = 0;
    cpu.memory[0] = OPCODE_ROL;
    cpu.memory[1] = MODE_REGISTER;
    cpu.memory[2] = 0x00;
    TEST_ASSERT_EQUAL_INT(CPU_OK, cpu_step(&cpu));
    TEST_ASSERT_EQUAL_UINT8(0x81, cpu.A);  // 10000001 (carry became bit 0)
    TEST_ASSERT_FALSE(cpu.flags & FLAG_CARRY);  // Bit 7 was 0
    TEST_ASSERT_FALSE(cpu.flags & FLAG_ZERO);
    TEST_ASSERT_TRUE(cpu.flags & FLAG_NEGATIVE);

    // Test 3: ROL MODE_REGISTER - Rotate with MSB set (carry out)
    initCPU(&cpu);
    cpu.A = 0xC3;  // 11000011
    cpu.flags = 0;  // Clear carry
    cpu.PC = 0;
    cpu.memory[0] = OPCODE_ROL;
    cpu.memory[1] = MODE_REGISTER;
    cpu.memory[2] = 0x00;
    TEST_ASSERT_EQUAL_INT(CPU_OK, cpu_step(&cpu));
    TEST_ASSERT_EQUAL_UINT8(0x86, cpu.A);  // 10000110
    TEST_ASSERT_TRUE(cpu.flags & FLAG_CARRY);  // Bit 7 was 1
    TEST_ASSERT_FALSE(cpu.flags & FLAG_ZERO);
    TEST_ASSERT_TRUE(cpu.flags & FLAG_NEGATIVE);

    // Test 4: ROL MODE_REGISTER - Result is zero
    initCPU(&cpu);
    cpu.A = 0x80;  // 10000000
    cpu.flags = 0;  // Clear carry
    cpu.PC = 0;
    cpu.memory[0] = OPCODE_ROL;
    cpu.memory[1] = MODE_REGISTER;
    cpu.memory[2] = 0x00;
    TEST_ASSERT_EQUAL_INT(CPU_OK, cpu_step(&cpu));
    TEST_ASSERT_EQUAL_UINT8(0x00, cpu.A);  // 00000000
    TEST_ASSERT_TRUE(cpu.flags & FLAG_CARRY);  // Bit 7 was 1
    TEST_ASSERT_TRUE(cpu.flags & FLAG_ZERO);
    TEST_ASSERT_FALSE(cpu.flags & FLAG_NEGATIVE);

    // Test 5: ROL MODE_ABSOLUTE - Rotate memory location
    initCPU(&cpu);
    cpu.memory[0x50] = 0x21;  // 00100001
    cpu.flags = 0;  // Clear carry
    cpu.PC = 0;
    cpu.memory[0] = OPCODE_ROL;
    cpu.memory[1] = MODE_ABSOLUTE;
    cpu.memory[2] = 0x50;
    TEST_ASSERT_EQUAL_INT(CPU_OK, cpu_step(&cpu));
    TEST_ASSERT_EQUAL_UINT8(0x42, cpu.memory[0x50]);  // 01000010
    TEST_ASSERT_FALSE(cpu.flags & FLAG_CARRY);  // Bit 7 was 0
    TEST_ASSERT_FALSE(cpu.flags & FLAG_ZERO);
    TEST_ASSERT_FALSE(cpu.flags & FLAG_NEGATIVE);

    // Test 6: ROL MODE_ABSOLUTE_X - Rotate memory location with X offset
    initCPU(&cpu);
    cpu.X = 0x05;
    cpu.memory[0x55] = 0xE1;  // 11100001
    cpu.flags = 0;  // Clear carry
    cpu.PC = 0;
    cpu.memory[0] = OPCODE_ROL;
    cpu.memory[1] = MODE_ABSOLUTE_X;
    cpu.memory[2] = 0x50;  // 0x50 + 0x05 = 0x55
    TEST_ASSERT_EQUAL_INT(CPU_OK, cpu_step(&cpu));
    TEST_ASSERT_EQUAL_UINT8(0xC2, cpu.memory[0x55]);  // 11000010
    TEST_ASSERT_TRUE(cpu.flags & FLAG_CARRY);  // Bit 7 was 1
    TEST_ASSERT_FALSE(cpu.flags & FLAG_ZERO);
    TEST_ASSERT_TRUE(cpu.flags & FLAG_NEGATIVE);

    // Test 7: ROL MODE_INDIRECT - Rotate indirect memory location
    initCPU(&cpu);
    cpu.memory[0x30] = 0x60;  // Indirect address
    cpu.memory[0x60] = 0x7F;  // 01111111
    cpu.flags = FLAG_CARRY;  // Set carry
    cpu.PC = 0;
    cpu.memory[0] = OPCODE_ROL;
    cpu.memory[1] = MODE_INDIRECT;
    cpu.memory[2] = 0x30;
    TEST_ASSERT_EQUAL_INT(CPU_OK, cpu_step(&cpu));
    TEST_ASSERT_EQUAL_UINT8(0xFF, cpu.memory[0x60]);  // 11111111 (carry in)
    TEST_ASSERT_FALSE(cpu.flags & FLAG_CARRY);  // Bit 7 was 0
    TEST_ASSERT_FALSE(cpu.flags & FLAG_ZERO);
    TEST_ASSERT_TRUE(cpu.flags & FLAG_NEGATIVE);

    // Test 8: ROL MODE_INDIRECT_X - Rotate indirect indexed memory location
    initCPU(&cpu);
    cpu.X = 0x02;
    cpu.memory[0x32] = 0x70;  // 0x30 + 0x02 = 0x32 (indirect address)
    cpu.memory[0x70] = 0x80;  // 10000000
    cpu.flags = FLAG_CARRY;  // Set carry
    cpu.PC = 0;
    cpu.memory[0] = OPCODE_ROL;
    cpu.memory[1] = MODE_INDIRECT_X;
    cpu.memory[2] = 0x30;
    TEST_ASSERT_EQUAL_INT(CPU_OK, cpu_step(&cpu));
    TEST_ASSERT_EQUAL_UINT8(0x01, cpu.memory[0x70]);  // 00000001 (carry in)
    TEST_ASSERT_TRUE(cpu.flags & FLAG_CARRY);  // Bit 7 was 1
    TEST_ASSERT_FALSE(cpu.flags & FLAG_ZERO);
    TEST_ASSERT_FALSE(cpu.flags & FLAG_NEGATIVE);

    // Test 9: ROL with invalid addressing mode
    initCPU(&cpu);
    cpu.PC = 0;
    cpu.memory[0] = OPCODE_ROL;
    cpu.memory[1] = 0xFF;  // Invalid mode
    cpu.memory[2] = 0x00;
    TEST_ASSERT_EQUAL_INT(CPU_ERROR, cpu_step(&cpu));
    TEST_ASSERT_TRUE(cpu.flags & FLAG_ERROR);

    // Test 10: ROL chain test - multiple rotations
    initCPU(&cpu);
    cpu.A = 0x01;  // 00000001
    cpu.flags = 0;  // Clear carry
    
    // First rotation: 0x01 -> 0x02, carry = 0
    cpu.PC = 0;
    cpu.memory[0] = OPCODE_ROL;
    cpu.memory[1] = MODE_REGISTER;
    cpu.memory[2] = 0x00;
    TEST_ASSERT_EQUAL_INT(CPU_OK, cpu_step(&cpu));
    TEST_ASSERT_EQUAL_UINT8(0x02, cpu.A);
    TEST_ASSERT_FALSE(cpu.flags & FLAG_CARRY);
    
    // Second rotation: 0x02 -> 0x04, carry = 0
    cpu.PC = 3;
    cpu.memory[3] = OPCODE_ROL;
    cpu.memory[4] = MODE_REGISTER;
    cpu.memory[5] = 0x00;
    TEST_ASSERT_EQUAL_INT(CPU_OK, cpu_step(&cpu));
    TEST_ASSERT_EQUAL_UINT8(0x04, cpu.A);
    TEST_ASSERT_FALSE(cpu.flags & FLAG_CARRY);

    // Test 11: ROL full 9-bit rotation through carry
    initCPU(&cpu);
    cpu.A = 0xFF;  // 11111111
    cpu.flags = FLAG_CARRY;  // Set carry
    cpu.PC = 0;
    cpu.memory[0] = OPCODE_ROL;
    cpu.memory[1] = MODE_REGISTER;
    cpu.memory[2] = 0x00;
    TEST_ASSERT_EQUAL_INT(CPU_OK, cpu_step(&cpu));
    TEST_ASSERT_EQUAL_UINT8(0xFF, cpu.A);  // 11111111 (carry in, carry out)
    TEST_ASSERT_TRUE(cpu.flags & FLAG_CARRY);  // Bit 7 was 1
    TEST_ASSERT_FALSE(cpu.flags & FLAG_ZERO);
    TEST_ASSERT_TRUE(cpu.flags & FLAG_NEGATIVE);
}
