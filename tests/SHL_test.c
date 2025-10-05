#include "unity/unity.h"
#include "../cpu.h"

void SHL_test(void) {
    CPU cpu;
    
    // Test 1: SHL MODE_REGISTER - Shift accumulator left (no carry out)
    initCPU(&cpu);
    cpu.A = 0x21;  // 00100001
    cpu.PC = 0;
    cpu.memory[0] = OPCODE_SHL;
    cpu.memory[1] = MODE_REGISTER;
    cpu.memory[2] = 0x00;  // Operand unused
    TEST_ASSERT_EQUAL_INT(CPU_OK, cpu_step(&cpu));
    TEST_ASSERT_EQUAL_UINT8(0x42, cpu.A);  // 01000010
    TEST_ASSERT_FALSE(cpu.flags & FLAG_CARRY);  // Bit 7 was 0
    TEST_ASSERT_FALSE(cpu.flags & FLAG_ZERO);
    TEST_ASSERT_FALSE(cpu.flags & FLAG_NEGATIVE);

    // Test 2: SHL MODE_REGISTER - Shift accumulator left (carry out)
    initCPU(&cpu);
    cpu.A = 0xC3;  // 11000011
    cpu.PC = 0;
    cpu.memory[0] = OPCODE_SHL;
    cpu.memory[1] = MODE_REGISTER;
    cpu.memory[2] = 0x00;
    TEST_ASSERT_EQUAL_INT(CPU_OK, cpu_step(&cpu));
    TEST_ASSERT_EQUAL_UINT8(0x86, cpu.A);  // 10000110
    TEST_ASSERT_TRUE(cpu.flags & FLAG_CARRY);  // Bit 7 was 1
    TEST_ASSERT_FALSE(cpu.flags & FLAG_ZERO);
    TEST_ASSERT_TRUE(cpu.flags & FLAG_NEGATIVE);

    // Test 3: SHL MODE_REGISTER - Result is zero
    initCPU(&cpu);
    cpu.A = 0x80;  // 10000000
    cpu.PC = 0;
    cpu.memory[0] = OPCODE_SHL;
    cpu.memory[1] = MODE_REGISTER;
    cpu.memory[2] = 0x00;
    TEST_ASSERT_EQUAL_INT(CPU_OK, cpu_step(&cpu));
    TEST_ASSERT_EQUAL_UINT8(0x00, cpu.A);  // 00000000
    TEST_ASSERT_TRUE(cpu.flags & FLAG_CARRY);  // Bit 7 was 1
    TEST_ASSERT_TRUE(cpu.flags & FLAG_ZERO);
    TEST_ASSERT_FALSE(cpu.flags & FLAG_NEGATIVE);

    // Test 4: SHL MODE_REGISTER - Shift positive number
    initCPU(&cpu);
    cpu.A = 0x7F;  // 01111111
    cpu.PC = 0;
    cpu.memory[0] = OPCODE_SHL;
    cpu.memory[1] = MODE_REGISTER;
    cpu.memory[2] = 0x00;
    TEST_ASSERT_EQUAL_INT(CPU_OK, cpu_step(&cpu));
    TEST_ASSERT_EQUAL_UINT8(0xFE, cpu.A);  // 11111110 (0 shifted into LSB)
    TEST_ASSERT_FALSE(cpu.flags & FLAG_CARRY);  // Bit 7 was 0
    TEST_ASSERT_FALSE(cpu.flags & FLAG_ZERO);
    TEST_ASSERT_TRUE(cpu.flags & FLAG_NEGATIVE);  // Result is negative

    // Test 5: SHL MODE_ABSOLUTE - Shift memory location
    initCPU(&cpu);
    cpu.memory[0x50] = 0x21;  // 00100001
    cpu.PC = 0;
    cpu.memory[0] = OPCODE_SHL;
    cpu.memory[1] = MODE_ABSOLUTE;
    cpu.memory[2] = 0x50;
    TEST_ASSERT_EQUAL_INT(CPU_OK, cpu_step(&cpu));
    TEST_ASSERT_EQUAL_UINT8(0x42, cpu.memory[0x50]);  // 01000010
    TEST_ASSERT_FALSE(cpu.flags & FLAG_CARRY);  // Bit 7 was 0
    TEST_ASSERT_FALSE(cpu.flags & FLAG_ZERO);
    TEST_ASSERT_FALSE(cpu.flags & FLAG_NEGATIVE);

    // Test 6: SHL MODE_ABSOLUTE_X - Shift memory location with X offset
    initCPU(&cpu);
    cpu.X = 0x05;
    cpu.memory[0x55] = 0xE1;  // 11100001
    cpu.PC = 0;
    cpu.memory[0] = OPCODE_SHL;
    cpu.memory[1] = MODE_ABSOLUTE_X;
    cpu.memory[2] = 0x50;  // 0x50 + 0x05 = 0x55
    TEST_ASSERT_EQUAL_INT(CPU_OK, cpu_step(&cpu));
    TEST_ASSERT_EQUAL_UINT8(0xC2, cpu.memory[0x55]);  // 11000010
    TEST_ASSERT_TRUE(cpu.flags & FLAG_CARRY);  // Bit 7 was 1
    TEST_ASSERT_FALSE(cpu.flags & FLAG_ZERO);
    TEST_ASSERT_TRUE(cpu.flags & FLAG_NEGATIVE);

    // Test 7: SHL MODE_INDIRECT - Shift indirect memory location
    initCPU(&cpu);
    cpu.memory[0x30] = 0x60;  // Indirect address
    cpu.memory[0x60] = 0x7F;  // 01111111
    cpu.PC = 0;
    cpu.memory[0] = OPCODE_SHL;
    cpu.memory[1] = MODE_INDIRECT;
    cpu.memory[2] = 0x30;
    TEST_ASSERT_EQUAL_INT(CPU_OK, cpu_step(&cpu));
    TEST_ASSERT_EQUAL_UINT8(0xFE, cpu.memory[0x60]);  // 11111110
    TEST_ASSERT_FALSE(cpu.flags & FLAG_CARRY);  // Bit 7 was 0
    TEST_ASSERT_FALSE(cpu.flags & FLAG_ZERO);
    TEST_ASSERT_TRUE(cpu.flags & FLAG_NEGATIVE);

    // Test 8: SHL MODE_INDIRECT_X - Shift indirect indexed memory location
    initCPU(&cpu);
    cpu.X = 0x02;
    cpu.memory[0x32] = 0x70;  // 0x30 + 0x02 = 0x32 (indirect address)
    cpu.memory[0x70] = 0x89;  // 10001001
    cpu.PC = 0;
    cpu.memory[0] = OPCODE_SHL;
    cpu.memory[1] = MODE_INDIRECT_X;
    cpu.memory[2] = 0x30;
    TEST_ASSERT_EQUAL_INT(CPU_OK, cpu_step(&cpu));
    TEST_ASSERT_EQUAL_UINT8(0x12, cpu.memory[0x70]);  // 00010010
    TEST_ASSERT_TRUE(cpu.flags & FLAG_CARRY);  // Bit 7 was 1
    TEST_ASSERT_FALSE(cpu.flags & FLAG_ZERO);
    TEST_ASSERT_FALSE(cpu.flags & FLAG_NEGATIVE);

    // Test 9: SHL with invalid addressing mode
    initCPU(&cpu);
    cpu.PC = 0;
    cpu.memory[0] = OPCODE_SHL;
    cpu.memory[1] = 0xFF;  // Invalid mode
    cpu.memory[2] = 0x00;
    TEST_ASSERT_EQUAL_INT(CPU_ERROR, cpu_step(&cpu));
    TEST_ASSERT_TRUE(cpu.flags & FLAG_ERROR);

    // Test 10: SHL chain test - multiply by powers of 2
    initCPU(&cpu);
    cpu.A = 0x01;  // 1
    
    // First shift: 1 -> 2
    cpu.PC = 0;
    cpu.memory[0] = OPCODE_SHL;
    cpu.memory[1] = MODE_REGISTER;
    cpu.memory[2] = 0x00;
    TEST_ASSERT_EQUAL_INT(CPU_OK, cpu_step(&cpu));
    TEST_ASSERT_EQUAL_UINT8(0x02, cpu.A);  // 2
    TEST_ASSERT_FALSE(cpu.flags & FLAG_CARRY);
    
    // Second shift: 2 -> 4
    cpu.PC = 3;
    cpu.memory[3] = OPCODE_SHL;
    cpu.memory[4] = MODE_REGISTER;
    cpu.memory[5] = 0x00;
    TEST_ASSERT_EQUAL_INT(CPU_OK, cpu_step(&cpu));
    TEST_ASSERT_EQUAL_UINT8(0x04, cpu.A);  // 4
    TEST_ASSERT_FALSE(cpu.flags & FLAG_CARRY);
    
    // Third shift: 4 -> 8
    cpu.PC = 6;
    cpu.memory[6] = OPCODE_SHL;
    cpu.memory[7] = MODE_REGISTER;
    cpu.memory[8] = 0x00;
    TEST_ASSERT_EQUAL_INT(CPU_OK, cpu_step(&cpu));
    TEST_ASSERT_EQUAL_UINT8(0x08, cpu.A);  // 8
    TEST_ASSERT_FALSE(cpu.flags & FLAG_CARRY);

    // Test 11: SHL with zero input
    initCPU(&cpu);
    cpu.A = 0x00;  // 00000000
    cpu.PC = 0;
    cpu.memory[0] = OPCODE_SHL;
    cpu.memory[1] = MODE_REGISTER;
    cpu.memory[2] = 0x00;
    TEST_ASSERT_EQUAL_INT(CPU_OK, cpu_step(&cpu));
    TEST_ASSERT_EQUAL_UINT8(0x00, cpu.A);  // 00000000
    TEST_ASSERT_FALSE(cpu.flags & FLAG_CARRY);  // Bit 7 was 0
    TEST_ASSERT_TRUE(cpu.flags & FLAG_ZERO);
    TEST_ASSERT_FALSE(cpu.flags & FLAG_NEGATIVE);

    // Test 12: SHL with value 0x40 (no overflow)
    initCPU(&cpu);
    cpu.A = 0x40;  // 01000000
    cpu.PC = 0;
    cpu.memory[0] = OPCODE_SHL;
    cpu.memory[1] = MODE_REGISTER;
    cpu.memory[2] = 0x00;
    TEST_ASSERT_EQUAL_INT(CPU_OK, cpu_step(&cpu));
    TEST_ASSERT_EQUAL_UINT8(0x80, cpu.A);  // 10000000
    TEST_ASSERT_FALSE(cpu.flags & FLAG_CARRY);  // Bit 7 was 0
    TEST_ASSERT_FALSE(cpu.flags & FLAG_ZERO);
    TEST_ASSERT_TRUE(cpu.flags & FLAG_NEGATIVE);

    // Test 13: SHL consecutive operations for overflow detection
    initCPU(&cpu);
    cpu.A = 0x40;  // 01000000
    
    // First shift: 0x40 -> 0x80 (no carry)
    cpu.PC = 0;
    cpu.memory[0] = OPCODE_SHL;
    cpu.memory[1] = MODE_REGISTER;
    cpu.memory[2] = 0x00;
    TEST_ASSERT_EQUAL_INT(CPU_OK, cpu_step(&cpu));
    TEST_ASSERT_EQUAL_UINT8(0x80, cpu.A);
    TEST_ASSERT_FALSE(cpu.flags & FLAG_CARRY);
    
    // Second shift: 0x80 -> 0x00 (carry out)
    cpu.PC = 3;
    cpu.memory[3] = OPCODE_SHL;
    cpu.memory[4] = MODE_REGISTER;
    cpu.memory[5] = 0x00;
    TEST_ASSERT_EQUAL_INT(CPU_OK, cpu_step(&cpu));
    TEST_ASSERT_EQUAL_UINT8(0x00, cpu.A);
    TEST_ASSERT_TRUE(cpu.flags & FLAG_CARRY);
    TEST_ASSERT_TRUE(cpu.flags & FLAG_ZERO);
}
