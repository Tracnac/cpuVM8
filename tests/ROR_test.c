#include "unity/unity.h"
#include "../cpu.h"

void ROR_test(void) {
    CPU cpu;
    
    // Test 1: ROR MODE_REGISTER - Rotate accumulator right without carry
    initCPU(&cpu);
    cpu.A = 0x42;  // 01000010
    cpu.flags = 0;  // Clear carry
    cpu.PC = 0;
    cpu.memory[0] = OPCODE_ROR;
    cpu.memory[1] = MODE_REGISTER;
    cpu.memory[2] = 0x00;  // Operand unused
    TEST_ASSERT_EQUAL_INT(CPU_OK, cpu_step(&cpu));
    TEST_ASSERT_EQUAL_UINT8(0x21, cpu.A);  // 00100001
    TEST_ASSERT_FALSE(cpu.flags & FLAG_CARRY);  // Bit 0 was 0
    TEST_ASSERT_FALSE(cpu.flags & FLAG_ZERO);
    TEST_ASSERT_FALSE(cpu.flags & FLAG_NEGATIVE);

    // Test 2: ROR MODE_REGISTER - Rotate accumulator right with carry in
    initCPU(&cpu);
    cpu.A = 0x40;  // 01000000
    cpu.flags = FLAG_CARRY;  // Set carry
    cpu.PC = 0;
    cpu.memory[0] = OPCODE_ROR;
    cpu.memory[1] = MODE_REGISTER;
    cpu.memory[2] = 0x00;
    TEST_ASSERT_EQUAL_INT(CPU_OK, cpu_step(&cpu));
    TEST_ASSERT_EQUAL_UINT8(0xA0, cpu.A);  // 10100000 (carry became bit 7)
    TEST_ASSERT_FALSE(cpu.flags & FLAG_CARRY);  // Bit 0 was 0
    TEST_ASSERT_FALSE(cpu.flags & FLAG_ZERO);
    TEST_ASSERT_TRUE(cpu.flags & FLAG_NEGATIVE);

    // Test 3: ROR MODE_REGISTER - Rotate odd number (carry out)
    initCPU(&cpu);
    cpu.A = 0x43;  // 01000011
    cpu.flags = 0;  // Clear carry
    cpu.PC = 0;
    cpu.memory[0] = OPCODE_ROR;
    cpu.memory[1] = MODE_REGISTER;
    cpu.memory[2] = 0x00;
    TEST_ASSERT_EQUAL_INT(CPU_OK, cpu_step(&cpu));
    TEST_ASSERT_EQUAL_UINT8(0x21, cpu.A);  // 00100001
    TEST_ASSERT_TRUE(cpu.flags & FLAG_CARRY);  // Bit 0 was 1
    TEST_ASSERT_FALSE(cpu.flags & FLAG_ZERO);
    TEST_ASSERT_FALSE(cpu.flags & FLAG_NEGATIVE);

    // Test 4: ROR MODE_REGISTER - Result is zero
    initCPU(&cpu);
    cpu.A = 0x01;  // 00000001
    cpu.flags = 0;  // Clear carry
    cpu.PC = 0;
    cpu.memory[0] = OPCODE_ROR;
    cpu.memory[1] = MODE_REGISTER;
    cpu.memory[2] = 0x00;
    TEST_ASSERT_EQUAL_INT(CPU_OK, cpu_step(&cpu));
    TEST_ASSERT_EQUAL_UINT8(0x00, cpu.A);  // 00000000
    TEST_ASSERT_TRUE(cpu.flags & FLAG_CARRY);  // Bit 0 was 1
    TEST_ASSERT_TRUE(cpu.flags & FLAG_ZERO);
    TEST_ASSERT_FALSE(cpu.flags & FLAG_NEGATIVE);

    // Test 5: ROR MODE_ABSOLUTE - Rotate memory location
    initCPU(&cpu);
    cpu.memory[0x50] = 0x84;  // 10000100
    cpu.flags = 0;  // Clear carry
    cpu.PC = 0;
    cpu.memory[0] = OPCODE_ROR;
    cpu.memory[1] = MODE_ABSOLUTE;
    cpu.memory[2] = 0x50;
    TEST_ASSERT_EQUAL_INT(CPU_OK, cpu_step(&cpu));
    TEST_ASSERT_EQUAL_UINT8(0x42, cpu.memory[0x50]);  // 01000010
    TEST_ASSERT_FALSE(cpu.flags & FLAG_CARRY);  // Bit 0 was 0
    TEST_ASSERT_FALSE(cpu.flags & FLAG_ZERO);
    TEST_ASSERT_FALSE(cpu.flags & FLAG_NEGATIVE);

    // Test 6: ROR MODE_ABSOLUTE_X - Rotate memory location with X offset
    initCPU(&cpu);
    cpu.X = 0x05;
    cpu.memory[0x55] = 0x87;  // 10000111
    cpu.flags = 0;  // Clear carry
    cpu.PC = 0;
    cpu.memory[0] = OPCODE_ROR;
    cpu.memory[1] = MODE_ABSOLUTE_X;
    cpu.memory[2] = 0x50;  // 0x50 + 0x05 = 0x55
    TEST_ASSERT_EQUAL_INT(CPU_OK, cpu_step(&cpu));
    TEST_ASSERT_EQUAL_UINT8(0x43, cpu.memory[0x55]);  // 01000011
    TEST_ASSERT_TRUE(cpu.flags & FLAG_CARRY);  // Bit 0 was 1
    TEST_ASSERT_FALSE(cpu.flags & FLAG_ZERO);
    TEST_ASSERT_FALSE(cpu.flags & FLAG_NEGATIVE);

    // Test 7: ROR MODE_INDIRECT - Rotate indirect memory location
    initCPU(&cpu);
    cpu.memory[0x30] = 0x60;  // Indirect address
    cpu.memory[0x60] = 0xFE;  // 11111110
    cpu.flags = FLAG_CARRY;  // Set carry
    cpu.PC = 0;
    cpu.memory[0] = OPCODE_ROR;
    cpu.memory[1] = MODE_INDIRECT;
    cpu.memory[2] = 0x30;
    TEST_ASSERT_EQUAL_INT(CPU_OK, cpu_step(&cpu));
    TEST_ASSERT_EQUAL_UINT8(0xFF, cpu.memory[0x60]);  // 11111111 (carry in)
    TEST_ASSERT_FALSE(cpu.flags & FLAG_CARRY);  // Bit 0 was 0
    TEST_ASSERT_FALSE(cpu.flags & FLAG_ZERO);
    TEST_ASSERT_TRUE(cpu.flags & FLAG_NEGATIVE);

    // Test 8: ROR MODE_INDIRECT_X - Rotate indirect indexed memory location
    initCPU(&cpu);
    cpu.X = 0x02;
    cpu.memory[0x32] = 0x70;  // 0x30 + 0x02 = 0x32 (indirect address)
    cpu.memory[0x70] = 0x01;  // 00000001
    cpu.flags = FLAG_CARRY;  // Set carry
    cpu.PC = 0;
    cpu.memory[0] = OPCODE_ROR;
    cpu.memory[1] = MODE_INDIRECT_X;
    cpu.memory[2] = 0x30;
    TEST_ASSERT_EQUAL_INT(CPU_OK, cpu_step(&cpu));
    TEST_ASSERT_EQUAL_UINT8(0x80, cpu.memory[0x70]);  // 10000000 (carry in)
    TEST_ASSERT_TRUE(cpu.flags & FLAG_CARRY);  // Bit 0 was 1
    TEST_ASSERT_FALSE(cpu.flags & FLAG_ZERO);
    TEST_ASSERT_TRUE(cpu.flags & FLAG_NEGATIVE);

    // Test 9: ROR with invalid addressing mode
    initCPU(&cpu);
    cpu.PC = 0;
    cpu.memory[0] = OPCODE_ROR;
    cpu.memory[1] = 0xFF;  // Invalid mode
    cpu.memory[2] = 0x00;
    TEST_ASSERT_EQUAL_INT(CPU_HALTED, cpu_step(&cpu));
    TEST_ASSERT_TRUE(cpu.flags & FLAG_HALTED);

    // Test 10: ROR chain test - multiple rotations
    initCPU(&cpu);
    cpu.A = 0x80;  // 10000000
    cpu.flags = 0;  // Clear carry
    
    // First rotation: 0x80 -> 0x40, carry = 0
    cpu.PC = 0;
    cpu.memory[0] = OPCODE_ROR;
    cpu.memory[1] = MODE_REGISTER;
    cpu.memory[2] = 0x00;
    TEST_ASSERT_EQUAL_INT(CPU_OK, cpu_step(&cpu));
    TEST_ASSERT_EQUAL_UINT8(0x40, cpu.A);
    TEST_ASSERT_FALSE(cpu.flags & FLAG_CARRY);
    
    // Second rotation: 0x40 -> 0x20, carry = 0
    cpu.PC = 3;
    cpu.memory[3] = OPCODE_ROR;
    cpu.memory[4] = MODE_REGISTER;
    cpu.memory[5] = 0x00;
    TEST_ASSERT_EQUAL_INT(CPU_OK, cpu_step(&cpu));
    TEST_ASSERT_EQUAL_UINT8(0x20, cpu.A);
    TEST_ASSERT_FALSE(cpu.flags & FLAG_CARRY);
}
