#include "unity/unity.h"
#include "../cpu.h"

void CPX_test(void) {
    CPU cpu;
    
    // Test 1: CPX immediate - equal values (X = operand)
    initCPU(&cpu);
    cpu.PC = 0;
    cpu.X = 0x42;
    cpu.memory[0] = OPCODE_CPX;
    cpu.memory[1] = MODE_IMMEDIAT;
    cpu.memory[2] = 0x42;
    TEST_ASSERT_EQUAL_INT(CPU_OK, cpu_step(&cpu));
    TEST_ASSERT_EQUAL_UINT8(0x42, cpu.X);  // X should be unchanged
    TEST_ASSERT_TRUE(cpu.flags & FLAG_ZERO);     // Z=1 (equal)
    TEST_ASSERT_TRUE(cpu.flags & FLAG_CARRY);    // C=1 (no borrow)
    TEST_ASSERT_FALSE(cpu.flags & FLAG_NEGATIVE); // N=0 (result is 0)
    TEST_ASSERT_FALSE(cpu.flags & FLAG_OVERFLOW); // V=0
    
    // Test 2: CPX immediate - X > operand
    initCPU(&cpu);
    cpu.PC = 0;
    cpu.X = 0x60;
    cpu.memory[0] = OPCODE_CPX;
    cpu.memory[1] = MODE_IMMEDIAT;
    cpu.memory[2] = 0x40;
    TEST_ASSERT_EQUAL_INT(CPU_OK, cpu_step(&cpu));
    TEST_ASSERT_EQUAL_UINT8(0x60, cpu.X);  // X should be unchanged
    TEST_ASSERT_FALSE(cpu.flags & FLAG_ZERO);     // Z=0 (not equal)
    TEST_ASSERT_TRUE(cpu.flags & FLAG_CARRY);     // C=1 (no borrow)
    TEST_ASSERT_FALSE(cpu.flags & FLAG_NEGATIVE); // N=0 (positive result)
    TEST_ASSERT_FALSE(cpu.flags & FLAG_OVERFLOW); // V=0
    
    // Test 3: CPX immediate - X < operand (borrow occurs)
    initCPU(&cpu);
    cpu.PC = 0;
    cpu.X = 0x30;
    cpu.memory[0] = OPCODE_CPX;
    cpu.memory[1] = MODE_IMMEDIAT;
    cpu.memory[2] = 0x50;
    TEST_ASSERT_EQUAL_INT(CPU_OK, cpu_step(&cpu));
    TEST_ASSERT_EQUAL_UINT8(0x30, cpu.X);  // X should be unchanged
    TEST_ASSERT_FALSE(cpu.flags & FLAG_ZERO);     // Z=0 (not equal)
    TEST_ASSERT_FALSE(cpu.flags & FLAG_CARRY);    // C=0 (borrow occurred)
    TEST_ASSERT_TRUE(cpu.flags & FLAG_NEGATIVE);  // N=1 (negative result)
    TEST_ASSERT_FALSE(cpu.flags & FLAG_OVERFLOW); // V=0
    
    // Test 4: CPX immediate - signed overflow case
    initCPU(&cpu);
    cpu.PC = 0;
    cpu.X = 0x80;  // -128 in signed
    cpu.memory[0] = OPCODE_CPX;
    cpu.memory[1] = MODE_IMMEDIAT;
    cpu.memory[2] = 0x01;  // +1 in signed
    TEST_ASSERT_EQUAL_INT(CPU_OK, cpu_step(&cpu));
    TEST_ASSERT_EQUAL_UINT8(0x80, cpu.X);  // X should be unchanged
    TEST_ASSERT_FALSE(cpu.flags & FLAG_ZERO);     // Z=0
    TEST_ASSERT_TRUE(cpu.flags & FLAG_CARRY);     // C=1 (no unsigned borrow)
    TEST_ASSERT_FALSE(cpu.flags & FLAG_NEGATIVE); // N=0 (0x7F is positive)
    TEST_ASSERT_TRUE(cpu.flags & FLAG_OVERFLOW);  // V=1 (signed overflow)
    
    // Test 5: CPX absolute addressing
    initCPU(&cpu);
    cpu.PC = 0;
    cpu.X = 0x33;
    cpu.memory[0] = OPCODE_CPX;
    cpu.memory[1] = MODE_ABSOLUTE;
    cpu.memory[2] = 0x10;
    cpu.memory[0x10] = 0x33;
    TEST_ASSERT_EQUAL_INT(CPU_OK, cpu_step(&cpu));
    TEST_ASSERT_EQUAL_UINT8(0x33, cpu.X);  // X should be unchanged
    TEST_ASSERT_TRUE(cpu.flags & FLAG_ZERO);      // Equal values
    TEST_ASSERT_TRUE(cpu.flags & FLAG_CARRY);     // No borrow
    TEST_ASSERT_FALSE(cpu.flags & FLAG_NEGATIVE);
    TEST_ASSERT_FALSE(cpu.flags & FLAG_OVERFLOW);
    
    // Test 6: CPX indexed addressing
    initCPU(&cpu);
    cpu.PC = 0;
    cpu.X = 0x77;
    cpu.memory[0] = OPCODE_CPX;
    cpu.memory[1] = MODE_INDEXED_X;
    cpu.memory[2] = 0x20;
    cpu.memory[0x97] = 0x88;  // 0x20 + 0x77 = 0x97
    TEST_ASSERT_EQUAL_INT(CPU_OK, cpu_step(&cpu));
    TEST_ASSERT_EQUAL_UINT8(0x77, cpu.X);  // X should be unchanged
    TEST_ASSERT_FALSE(cpu.flags & FLAG_ZERO);     // 0x77 != 0x88
    TEST_ASSERT_FALSE(cpu.flags & FLAG_CARRY);    // Borrow occurred
    TEST_ASSERT_TRUE(cpu.flags & FLAG_NEGATIVE);  // Negative result
    
    // Test 7: CPX indirect addressing
    initCPU(&cpu);
    cpu.PC = 0;
    cpu.X = 0x44;
    cpu.memory[0] = OPCODE_CPX;
    cpu.memory[1] = MODE_INDIRECT;
    cpu.memory[2] = 0x15;
    cpu.memory[0x15] = 0x50;  // Indirect address
    cpu.memory[0x50] = 0x44;  // Value to compare
    TEST_ASSERT_EQUAL_INT(CPU_OK, cpu_step(&cpu));
    TEST_ASSERT_EQUAL_UINT8(0x44, cpu.X);  // X should be unchanged
    TEST_ASSERT_TRUE(cpu.flags & FLAG_ZERO);      // Equal values
    TEST_ASSERT_TRUE(cpu.flags & FLAG_CARRY);
    TEST_ASSERT_FALSE(cpu.flags & FLAG_NEGATIVE);
    TEST_ASSERT_FALSE(cpu.flags & FLAG_OVERFLOW);
    
    // Test 8: CPX indirect indexed addressing
    initCPU(&cpu);
    cpu.PC = 0;
    cpu.X = 0x55;
    cpu.memory[0] = OPCODE_CPX;
    cpu.memory[1] = MODE_INDIRECT_INDEXED_X;
    cpu.memory[2] = 0x40;
    cpu.memory[0x95] = 0x60;  // 0x40 + 0x55 = 0x95 (indirect address)
    cpu.memory[0x60] = 0x55;  // Value to compare
    TEST_ASSERT_EQUAL_INT(CPU_OK, cpu_step(&cpu));
    TEST_ASSERT_EQUAL_UINT8(0x55, cpu.X);  // X should be unchanged
    TEST_ASSERT_TRUE(cpu.flags & FLAG_ZERO);      // Equal values
    TEST_ASSERT_TRUE(cpu.flags & FLAG_CARRY);
    TEST_ASSERT_FALSE(cpu.flags & FLAG_NEGATIVE);
    TEST_ASSERT_FALSE(cpu.flags & FLAG_OVERFLOW);
    
    // Test 9: CPX with invalid addressing mode
    initCPU(&cpu);
    cpu.PC = 0;
    cpu.X = 0x11;
    cpu.memory[0] = OPCODE_CPX;
    cpu.memory[1] = 0xFF;  // Invalid mode
    cpu.memory[2] = 0x22;
    TEST_ASSERT_EQUAL_INT(CPU_ERROR, cpu_step(&cpu));
    TEST_ASSERT_TRUE(cpu.flags & FLAG_ERROR);
    TEST_ASSERT_EQUAL_UINT8(0x11, cpu.X);  // X should be unchanged
    
    // Test 10: CPX with zero result (boundary case)
    initCPU(&cpu);
    cpu.PC = 0;
    cpu.X = 0x00;
    cpu.memory[0] = OPCODE_CPX;
    cpu.memory[1] = MODE_IMMEDIAT;
    cpu.memory[2] = 0x00;
    TEST_ASSERT_EQUAL_INT(CPU_OK, cpu_step(&cpu));
    TEST_ASSERT_EQUAL_UINT8(0x00, cpu.X);  // X should be unchanged
    TEST_ASSERT_TRUE(cpu.flags & FLAG_ZERO);
    TEST_ASSERT_TRUE(cpu.flags & FLAG_CARRY);     // No borrow
    TEST_ASSERT_FALSE(cpu.flags & FLAG_NEGATIVE);
    TEST_ASSERT_FALSE(cpu.flags & FLAG_OVERFLOW);
    
    // Test 11: CPX creating maximum negative result
    initCPU(&cpu);
    cpu.PC = 0;
    cpu.X = 0x00;
    cpu.memory[0] = OPCODE_CPX;
    cpu.memory[1] = MODE_IMMEDIAT;
    cpu.memory[2] = 0x01;
    TEST_ASSERT_EQUAL_INT(CPU_OK, cpu_step(&cpu));
    TEST_ASSERT_EQUAL_UINT8(0x00, cpu.X);  // X should be unchanged
    TEST_ASSERT_FALSE(cpu.flags & FLAG_ZERO);     // 0x00 - 0x01 = 0xFF
    TEST_ASSERT_FALSE(cpu.flags & FLAG_CARRY);    // Borrow occurred
    TEST_ASSERT_TRUE(cpu.flags & FLAG_NEGATIVE);  // 0xFF is negative
    TEST_ASSERT_FALSE(cpu.flags & FLAG_OVERFLOW);
    
    // Test 12: CPX that would cause SUB overflow (signed)
    initCPU(&cpu);
    cpu.PC = 0;
    cpu.X = 0x7F;  // +127 (maximum positive signed)
    cpu.memory[0] = OPCODE_CPX;
    cpu.memory[1] = MODE_IMMEDIAT;
    cpu.memory[2] = 0x80;  // -128 (minimum negative signed)
    TEST_ASSERT_EQUAL_INT(CPU_OK, cpu_step(&cpu));
    TEST_ASSERT_EQUAL_UINT8(0x7F, cpu.X);  // X should be unchanged
    TEST_ASSERT_FALSE(cpu.flags & FLAG_ZERO);
    TEST_ASSERT_FALSE(cpu.flags & FLAG_CARRY);    // Borrow in unsigned
    TEST_ASSERT_TRUE(cpu.flags & FLAG_NEGATIVE);  // Result is 0xFF
    TEST_ASSERT_TRUE(cpu.flags & FLAG_OVERFLOW);  // Signed overflow
    
    // Test 13: CPX loop counter scenario (X counting down)
    initCPU(&cpu);
    cpu.PC = 0;
    cpu.X = 0x01;  // Loop counter
    cpu.memory[0] = OPCODE_CPX;
    cpu.memory[1] = MODE_IMMEDIAT;
    cpu.memory[2] = 0x00;  // Compare with zero
    TEST_ASSERT_EQUAL_INT(CPU_OK, cpu_step(&cpu));
    TEST_ASSERT_EQUAL_UINT8(0x01, cpu.X);  // X should be unchanged
    TEST_ASSERT_FALSE(cpu.flags & FLAG_ZERO);     // X != 0
    TEST_ASSERT_TRUE(cpu.flags & FLAG_CARRY);     // X > 0
    TEST_ASSERT_FALSE(cpu.flags & FLAG_NEGATIVE);
    
    // Test 14: CPX array bounds checking scenario
    initCPU(&cpu);
    cpu.PC = 0;
    cpu.X = 0x10;  // Array index
    cpu.memory[0] = OPCODE_CPX;
    cpu.memory[1] = MODE_IMMEDIAT;
    cpu.memory[2] = 0x20;  // Array size
    TEST_ASSERT_EQUAL_INT(CPU_OK, cpu_step(&cpu));
    TEST_ASSERT_EQUAL_UINT8(0x10, cpu.X);  // X should be unchanged
    TEST_ASSERT_FALSE(cpu.flags & FLAG_ZERO);     // Index != size
    TEST_ASSERT_FALSE(cpu.flags & FLAG_CARRY);    // Index < size (borrow)
    TEST_ASSERT_TRUE(cpu.flags & FLAG_NEGATIVE);  // Negative result
    // This means X < array_size, so index is valid
    
    // Test 15: CPX maximum value comparison
    initCPU(&cpu);
    cpu.PC = 0;
    cpu.X = 0xFF;  // Maximum value
    cpu.memory[0] = OPCODE_CPX;
    cpu.memory[1] = MODE_IMMEDIAT;
    cpu.memory[2] = 0xFF;
    TEST_ASSERT_EQUAL_INT(CPU_OK, cpu_step(&cpu));
    TEST_ASSERT_EQUAL_UINT8(0xFF, cpu.X);  // X should be unchanged
    TEST_ASSERT_TRUE(cpu.flags & FLAG_ZERO);      // Equal
    TEST_ASSERT_TRUE(cpu.flags & FLAG_CARRY);
    TEST_ASSERT_FALSE(cpu.flags & FLAG_NEGATIVE);
    TEST_ASSERT_FALSE(cpu.flags & FLAG_OVERFLOW);
}
