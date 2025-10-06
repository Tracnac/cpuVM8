#include "unity/unity.h"
#include "../cpu.h"

void CMP_test(void) {
    CPU cpu;

    // Test 1: CMP immediate - equal values (A = operand)
    initCPU(&cpu);
    cpu.PC = 0;
    cpu.A = 0x42;
    cpu.memory[0] = OPCODE_CMP;
    cpu.memory[1] = MODE_IMMEDIAT;
    cpu.memory[2] = 0x42;
    TEST_ASSERT_EQUAL_INT(CPU_OK, cpu_step(&cpu));
    TEST_ASSERT_EQUAL_UINT8(0x42, cpu.A);  // A should be unchanged
    TEST_ASSERT_TRUE(cpu.flags & FLAG_ZERO);     // Z=1 (equal)
    TEST_ASSERT_TRUE(cpu.flags & FLAG_CARRY);    // C=1 (no borrow)
    TEST_ASSERT_FALSE(cpu.flags & FLAG_NEGATIVE); // N=0 (result is 0)
    TEST_ASSERT_FALSE(cpu.flags & FLAG_OVERFLOW); // V=0

    // Test 2: CMP immediate - A > operand
    initCPU(&cpu);
    cpu.PC = 0;
    cpu.A = 0x50;
    cpu.memory[0] = OPCODE_CMP;
    cpu.memory[1] = MODE_IMMEDIAT;
    cpu.memory[2] = 0x30;
    TEST_ASSERT_EQUAL_INT(CPU_OK, cpu_step(&cpu));
    TEST_ASSERT_EQUAL_UINT8(0x50, cpu.A);  // A should be unchanged
    TEST_ASSERT_FALSE(cpu.flags & FLAG_ZERO);     // Z=0 (not equal)
    TEST_ASSERT_TRUE(cpu.flags & FLAG_CARRY);     // C=1 (no borrow)
    TEST_ASSERT_FALSE(cpu.flags & FLAG_NEGATIVE); // N=0 (positive result)
    TEST_ASSERT_FALSE(cpu.flags & FLAG_OVERFLOW); // V=0

    // Test 3: CMP immediate - A < operand (borrow occurs)
    initCPU(&cpu);
    cpu.PC = 0;
    cpu.A = 0x30;
    cpu.memory[0] = OPCODE_CMP;
    cpu.memory[1] = MODE_IMMEDIAT;
    cpu.memory[2] = 0x50;
    TEST_ASSERT_EQUAL_INT(CPU_OK, cpu_step(&cpu));
    TEST_ASSERT_EQUAL_UINT8(0x30, cpu.A);  // A should be unchanged
    TEST_ASSERT_FALSE(cpu.flags & FLAG_ZERO);     // Z=0 (not equal)
    TEST_ASSERT_FALSE(cpu.flags & FLAG_CARRY);    // C=0 (borrow occurred)
    TEST_ASSERT_TRUE(cpu.flags & FLAG_NEGATIVE);  // N=1 (negative result)
    TEST_ASSERT_FALSE(cpu.flags & FLAG_OVERFLOW); // V=0

    // Test 4: CMP immediate - signed overflow case
    initCPU(&cpu);
    cpu.PC = 0;
    cpu.A = 0x80;  // -128 in signed
    cpu.memory[0] = OPCODE_CMP;
    cpu.memory[1] = MODE_IMMEDIAT;
    cpu.memory[2] = 0x01;  // +1 in signed
    TEST_ASSERT_EQUAL_INT(CPU_OK, cpu_step(&cpu));
    TEST_ASSERT_EQUAL_UINT8(0x80, cpu.A);  // A should be unchanged
    TEST_ASSERT_FALSE(cpu.flags & FLAG_ZERO);     // Z=0
    TEST_ASSERT_TRUE(cpu.flags & FLAG_CARRY);     // C=1 (no unsigned borrow)
    TEST_ASSERT_FALSE(cpu.flags & FLAG_NEGATIVE); // N=0 (0x7F is positive)
    TEST_ASSERT_TRUE(cpu.flags & FLAG_OVERFLOW);  // V=1 (signed overflow)

    // Test 5: CMP absolute addressing
    initCPU(&cpu);
    cpu.PC = 0;
    cpu.A = 0x25;
    cpu.memory[0] = OPCODE_CMP;
    cpu.memory[1] = MODE_ABSOLUTE;
    cpu.memory[2] = 0x10;
    cpu.memory[0x10] = 0x25;
    TEST_ASSERT_EQUAL_INT(CPU_OK, cpu_step(&cpu));
    TEST_ASSERT_EQUAL_UINT8(0x25, cpu.A);  // A should be unchanged
    TEST_ASSERT_TRUE(cpu.flags & FLAG_ZERO);      // Equal values
    TEST_ASSERT_TRUE(cpu.flags & FLAG_CARRY);     // No borrow
    TEST_ASSERT_FALSE(cpu.flags & FLAG_NEGATIVE);
    TEST_ASSERT_FALSE(cpu.flags & FLAG_OVERFLOW);

    // Test 6: CMP indexed addressing
    initCPU(&cpu);
    cpu.PC = 0;
    cpu.A = 0x77;
    cpu.X = 0x05;
    cpu.memory[0] = OPCODE_CMP;
    cpu.memory[1] = MODE_ABSOLUTE_X;
    cpu.memory[2] = 0x20;
    cpu.memory[0x25] = 0x88;  // 0x20 + 0x05 = 0x25
    TEST_ASSERT_EQUAL_INT(CPU_OK, cpu_step(&cpu));
    TEST_ASSERT_EQUAL_UINT8(0x77, cpu.A);  // A should be unchanged
    TEST_ASSERT_FALSE(cpu.flags & FLAG_ZERO);     // 0x77 != 0x88
    TEST_ASSERT_FALSE(cpu.flags & FLAG_CARRY);    // Borrow occurred
    TEST_ASSERT_TRUE(cpu.flags & FLAG_NEGATIVE);  // Negative result

    // Test 7: CMP indirect addressing
    initCPU(&cpu);
    cpu.PC = 0;
    cpu.A = 0x33;
    cpu.memory[0] = OPCODE_CMP;
    cpu.memory[1] = MODE_INDIRECT;
    cpu.memory[2] = 0x15;
    cpu.memory[0x15] = 0x40;  // Indirect address
    cpu.memory[0x40] = 0x33;  // Value to compare
    TEST_ASSERT_EQUAL_INT(CPU_OK, cpu_step(&cpu));
    TEST_ASSERT_EQUAL_UINT8(0x33, cpu.A);  // A should be unchanged
    TEST_ASSERT_TRUE(cpu.flags & FLAG_ZERO);      // Equal values
    TEST_ASSERT_TRUE(cpu.flags & FLAG_CARRY);
    TEST_ASSERT_FALSE(cpu.flags & FLAG_NEGATIVE);
    TEST_ASSERT_FALSE(cpu.flags & FLAG_OVERFLOW);

    // Test 8: CMP indirect indexed addressing
    initCPU(&cpu);
    cpu.PC = 0;
    cpu.A = 0x99;
    cpu.X = 0x03;
    cpu.memory[0] = OPCODE_CMP;
    cpu.memory[1] = MODE_INDIRECT_X;
    cpu.memory[2] = 0x50;
    cpu.memory[0x53] = 0x60;  // 0x50 + 0x03 = 0x53 (indirect address)
    cpu.memory[0x60] = 0x99;  // Value to compare
    TEST_ASSERT_EQUAL_INT(CPU_OK, cpu_step(&cpu));
    TEST_ASSERT_EQUAL_UINT8(0x99, cpu.A);  // A should be unchanged
    TEST_ASSERT_TRUE(cpu.flags & FLAG_ZERO);      // Equal values
    TEST_ASSERT_TRUE(cpu.flags & FLAG_CARRY);
    TEST_ASSERT_FALSE(cpu.flags & FLAG_NEGATIVE);
    TEST_ASSERT_FALSE(cpu.flags & FLAG_OVERFLOW);

    // Test 9: CMP with invalid addressing mode
    initCPU(&cpu);
    cpu.PC = 0;
    cpu.A = 0x11;
    cpu.memory[0] = OPCODE_CMP;
    cpu.memory[1] = 0xFF;  // Invalid mode
    cpu.memory[2] = 0x22;
    TEST_ASSERT_EQUAL_INT(CPU_HALTED, cpu_step(&cpu));
    TEST_ASSERT_TRUE(cpu.flags & FLAG_HALTED);
    TEST_ASSERT_EQUAL_UINT8(0x11, cpu.A);  // A should be unchanged

    // Test 10: CMP with zero result (boundary case)
    initCPU(&cpu);
    cpu.PC = 0;
    cpu.A = 0x00;
    cpu.memory[0] = OPCODE_CMP;
    cpu.memory[1] = MODE_IMMEDIAT;
    cpu.memory[2] = 0x00;
    TEST_ASSERT_EQUAL_INT(CPU_OK, cpu_step(&cpu));
    TEST_ASSERT_EQUAL_UINT8(0x00, cpu.A);  // A should be unchanged
    TEST_ASSERT_TRUE(cpu.flags & FLAG_ZERO);
    TEST_ASSERT_TRUE(cpu.flags & FLAG_CARRY);     // No borrow
    TEST_ASSERT_FALSE(cpu.flags & FLAG_NEGATIVE);
    TEST_ASSERT_FALSE(cpu.flags & FLAG_OVERFLOW);

    // Test 11: CMP creating maximum negative result
    initCPU(&cpu);
    cpu.PC = 0;
    cpu.A = 0x00;
    cpu.memory[0] = OPCODE_CMP;
    cpu.memory[1] = MODE_IMMEDIAT;
    cpu.memory[2] = 0x01;
    TEST_ASSERT_EQUAL_INT(CPU_OK, cpu_step(&cpu));
    TEST_ASSERT_EQUAL_UINT8(0x00, cpu.A);  // A should be unchanged
    TEST_ASSERT_FALSE(cpu.flags & FLAG_ZERO);     // 0x00 - 0x01 = 0xFF
    TEST_ASSERT_FALSE(cpu.flags & FLAG_CARRY);    // Borrow occurred
    TEST_ASSERT_TRUE(cpu.flags & FLAG_NEGATIVE);  // 0xFF is negative
    TEST_ASSERT_FALSE(cpu.flags & FLAG_OVERFLOW);

    // Test 12: CMP that would cause SUB overflow (signed)
    initCPU(&cpu);
    cpu.PC = 0;
    cpu.A = 0x7F;  // +127 (maximum positive signed)
    cpu.memory[0] = OPCODE_CMP;
    cpu.memory[1] = MODE_IMMEDIAT;
    cpu.memory[2] = 0x80;  // -128 (minimum negative signed)
    TEST_ASSERT_EQUAL_INT(CPU_OK, cpu_step(&cpu));
    TEST_ASSERT_EQUAL_UINT8(0x7F, cpu.A);  // A should be unchanged
    TEST_ASSERT_FALSE(cpu.flags & FLAG_ZERO);
    TEST_ASSERT_FALSE(cpu.flags & FLAG_CARRY);    // Borrow in unsigned
    TEST_ASSERT_TRUE(cpu.flags & FLAG_NEGATIVE);  // Result is 0xFF
    TEST_ASSERT_TRUE(cpu.flags & FLAG_OVERFLOW);  // Signed overflow
}
