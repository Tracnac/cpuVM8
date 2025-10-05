#include "unity/unity.h"
#include "../cpu.h"

void SHR_test(void) {
    CPU cpu;
    
    // Test 1: SHR MODE_REGISTER - Shift accumulator right (even number)
    initCPU(&cpu);
    cpu.A = 0x42;  // 01000010
    cpu.PC = 0;
    cpu.memory[0] = OPCODE_SHR;
    cpu.memory[1] = MODE_REGISTER;
    cpu.memory[2] = 0x00;  // Operand unused
    TEST_ASSERT_EQUAL_INT(CPU_OK, cpu_step(&cpu));
    TEST_ASSERT_EQUAL_UINT8(0x21, cpu.A);  // 00100001
    TEST_ASSERT_FALSE(cpu.flags & FLAG_CARRY);  // Bit 0 was 0
    TEST_ASSERT_FALSE(cpu.flags & FLAG_ZERO);
    TEST_ASSERT_FALSE(cpu.flags & FLAG_NEGATIVE);

    // Test 2: SHR MODE_REGISTER - Shift accumulator right (odd number)
    initCPU(&cpu);
    cpu.A = 0x43;  // 01000011
    cpu.PC = 0;
    cpu.memory[0] = OPCODE_SHR;
    cpu.memory[1] = MODE_REGISTER;
    cpu.memory[2] = 0x00;
    TEST_ASSERT_EQUAL_INT(CPU_OK, cpu_step(&cpu));
    TEST_ASSERT_EQUAL_UINT8(0x21, cpu.A);  // 00100001
    TEST_ASSERT_TRUE(cpu.flags & FLAG_CARRY);  // Bit 0 was 1
    TEST_ASSERT_FALSE(cpu.flags & FLAG_ZERO);
    TEST_ASSERT_FALSE(cpu.flags & FLAG_NEGATIVE);

    // Test 3: SHR MODE_REGISTER - Result is zero
    initCPU(&cpu);
    cpu.A = 0x01;  // 00000001
    cpu.PC = 0;
    cpu.memory[0] = OPCODE_SHR;
    cpu.memory[1] = MODE_REGISTER;
    cpu.memory[2] = 0x00;
    TEST_ASSERT_EQUAL_INT(CPU_OK, cpu_step(&cpu));
    TEST_ASSERT_EQUAL_UINT8(0x00, cpu.A);  // 00000000
    TEST_ASSERT_TRUE(cpu.flags & FLAG_CARRY);  // Bit 0 was 1
    TEST_ASSERT_TRUE(cpu.flags & FLAG_ZERO);
    TEST_ASSERT_FALSE(cpu.flags & FLAG_NEGATIVE);

    // Test 4: SHR MODE_REGISTER - Shift negative number
    initCPU(&cpu);
    cpu.A = 0xFF;  // 11111111
    cpu.PC = 0;
    cpu.memory[0] = OPCODE_SHR;
    cpu.memory[1] = MODE_REGISTER;
    cpu.memory[2] = 0x00;
    TEST_ASSERT_EQUAL_INT(CPU_OK, cpu_step(&cpu));
    TEST_ASSERT_EQUAL_UINT8(0x7F, cpu.A);  // 01111111 (0 shifted into MSB)
    TEST_ASSERT_TRUE(cpu.flags & FLAG_CARRY);  // Bit 0 was 1
    TEST_ASSERT_FALSE(cpu.flags & FLAG_ZERO);
    TEST_ASSERT_FALSE(cpu.flags & FLAG_NEGATIVE);  // Result is positive

    // Test 5: SHR MODE_ABSOLUTE - Shift memory location
    initCPU(&cpu);
    cpu.memory[0x50] = 0x84;  // 10000100
    cpu.PC = 0;
    cpu.memory[0] = OPCODE_SHR;
    cpu.memory[1] = MODE_ABSOLUTE;
    cpu.memory[2] = 0x50;
    TEST_ASSERT_EQUAL_INT(CPU_OK, cpu_step(&cpu));
    TEST_ASSERT_EQUAL_UINT8(0x42, cpu.memory[0x50]);  // 01000010
    TEST_ASSERT_FALSE(cpu.flags & FLAG_CARRY);  // Bit 0 was 0
    TEST_ASSERT_FALSE(cpu.flags & FLAG_ZERO);
    TEST_ASSERT_FALSE(cpu.flags & FLAG_NEGATIVE);

    // Test 6: SHR MODE_ABSOLUTE_X - Shift memory location with X offset
    initCPU(&cpu);
    cpu.X = 0x05;
    cpu.memory[0x55] = 0x87;  // 10000111
    cpu.PC = 0;
    cpu.memory[0] = OPCODE_SHR;
    cpu.memory[1] = MODE_ABSOLUTE_X;
    cpu.memory[2] = 0x50;  // 0x50 + 0x05 = 0x55
    TEST_ASSERT_EQUAL_INT(CPU_OK, cpu_step(&cpu));
    TEST_ASSERT_EQUAL_UINT8(0x43, cpu.memory[0x55]);  // 01000011
    TEST_ASSERT_TRUE(cpu.flags & FLAG_CARRY);  // Bit 0 was 1
    TEST_ASSERT_FALSE(cpu.flags & FLAG_ZERO);
    TEST_ASSERT_FALSE(cpu.flags & FLAG_NEGATIVE);

    // Test 7: SHR MODE_INDIRECT - Shift indirect memory location
    initCPU(&cpu);
    cpu.memory[0x30] = 0x60;  // Indirect address
    cpu.memory[0x60] = 0xFE;  // 11111110
    cpu.PC = 0;
    cpu.memory[0] = OPCODE_SHR;
    cpu.memory[1] = MODE_INDIRECT;
    cpu.memory[2] = 0x30;
    TEST_ASSERT_EQUAL_INT(CPU_OK, cpu_step(&cpu));
    TEST_ASSERT_EQUAL_UINT8(0x7F, cpu.memory[0x60]);  // 01111111
    TEST_ASSERT_FALSE(cpu.flags & FLAG_CARRY);  // Bit 0 was 0
    TEST_ASSERT_FALSE(cpu.flags & FLAG_ZERO);
    TEST_ASSERT_FALSE(cpu.flags & FLAG_NEGATIVE);

    // Test 8: SHR MODE_INDIRECT_X - Shift indirect indexed memory location
    initCPU(&cpu);
    cpu.X = 0x02;
    cpu.memory[0x32] = 0x70;  // 0x30 + 0x02 = 0x32 (indirect address)
    cpu.memory[0x70] = 0x89;  // 10001001
    cpu.PC = 0;
    cpu.memory[0] = OPCODE_SHR;
    cpu.memory[1] = MODE_INDIRECT_X;
    cpu.memory[2] = 0x30;
    TEST_ASSERT_EQUAL_INT(CPU_OK, cpu_step(&cpu));
    TEST_ASSERT_EQUAL_UINT8(0x44, cpu.memory[0x70]);  // 01000100
    TEST_ASSERT_TRUE(cpu.flags & FLAG_CARRY);  // Bit 0 was 1
    TEST_ASSERT_FALSE(cpu.flags & FLAG_ZERO);
    TEST_ASSERT_FALSE(cpu.flags & FLAG_NEGATIVE);

    // Test 9: SHR with invalid addressing mode
    initCPU(&cpu);
    cpu.PC = 0;
    cpu.memory[0] = OPCODE_SHR;
    cpu.memory[1] = 0xFF;  // Invalid mode
    cpu.memory[2] = 0x00;
    TEST_ASSERT_EQUAL_INT(CPU_ERROR, cpu_step(&cpu));
    TEST_ASSERT_TRUE(cpu.flags & FLAG_ERROR);

    // Test 10: SHR chain test - divide by powers of 2
    initCPU(&cpu);
    cpu.A = 0x80;  // 128
    
    // First shift: 128 -> 64
    cpu.PC = 0;
    cpu.memory[0] = OPCODE_SHR;
    cpu.memory[1] = MODE_REGISTER;
    cpu.memory[2] = 0x00;
    TEST_ASSERT_EQUAL_INT(CPU_OK, cpu_step(&cpu));
    TEST_ASSERT_EQUAL_UINT8(0x40, cpu.A);  // 64
    TEST_ASSERT_FALSE(cpu.flags & FLAG_CARRY);
    
    // Second shift: 64 -> 32
    cpu.PC = 3;
    cpu.memory[3] = OPCODE_SHR;
    cpu.memory[4] = MODE_REGISTER;
    cpu.memory[5] = 0x00;
    TEST_ASSERT_EQUAL_INT(CPU_OK, cpu_step(&cpu));
    TEST_ASSERT_EQUAL_UINT8(0x20, cpu.A);  // 32
    TEST_ASSERT_FALSE(cpu.flags & FLAG_CARRY);
    
    // Third shift: 32 -> 16
    cpu.PC = 6;
    cpu.memory[6] = OPCODE_SHR;
    cpu.memory[7] = MODE_REGISTER;
    cpu.memory[8] = 0x00;
    TEST_ASSERT_EQUAL_INT(CPU_OK, cpu_step(&cpu));
    TEST_ASSERT_EQUAL_UINT8(0x10, cpu.A);  // 16
    TEST_ASSERT_FALSE(cpu.flags & FLAG_CARRY);

    // Test 11: SHR with zero input
    initCPU(&cpu);
    cpu.A = 0x00;  // 00000000
    cpu.PC = 0;
    cpu.memory[0] = OPCODE_SHR;
    cpu.memory[1] = MODE_REGISTER;
    cpu.memory[2] = 0x00;
    TEST_ASSERT_EQUAL_INT(CPU_OK, cpu_step(&cpu));
    TEST_ASSERT_EQUAL_UINT8(0x00, cpu.A);  // 00000000
    TEST_ASSERT_FALSE(cpu.flags & FLAG_CARRY);  // Bit 0 was 0
    TEST_ASSERT_TRUE(cpu.flags & FLAG_ZERO);
    TEST_ASSERT_FALSE(cpu.flags & FLAG_NEGATIVE);

    // Test 12: SHR maximum value
    initCPU(&cpu);
    cpu.A = 0xFE;  // 11111110
    cpu.PC = 0;
    cpu.memory[0] = OPCODE_SHR;
    cpu.memory[1] = MODE_REGISTER;
    cpu.memory[2] = 0x00;
    TEST_ASSERT_EQUAL_INT(CPU_OK, cpu_step(&cpu));
    TEST_ASSERT_EQUAL_UINT8(0x7F, cpu.A);  // 01111111
    TEST_ASSERT_FALSE(cpu.flags & FLAG_CARRY);  // Bit 0 was 0
    TEST_ASSERT_FALSE(cpu.flags & FLAG_ZERO);
    TEST_ASSERT_FALSE(cpu.flags & FLAG_NEGATIVE);
}
