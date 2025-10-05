#include "unity/unity.h"
#include "../cpu.h"

void edge_cases_test(void) {
    CPU cpu;
    
    // Test 1: All opcodes 0x00-0x0F are now valid
    // Test CMP (0x0D)
    initCPU(&cpu);
    cpu.PC = 0;
    cpu.A = 0x42;
    cpu.memory[0] = 0x0D;  // CMP opcode (now valid)
    cpu.memory[1] = MODE_IMMEDIAT;  // Valid mode
    cpu.memory[2] = 0x42;  // Compare with same value
    TEST_ASSERT_EQUAL_INT(CPU_OK, cpu_step(&cpu));
    TEST_ASSERT_TRUE(cpu.flags & FLAG_ZERO);  // Should be equal
    
    // Test CPX (0x0E)
    initCPU(&cpu);
    cpu.PC = 0;
    cpu.X = 0x33;
    cpu.memory[0] = 0x0E;  // CPX opcode (now valid)
    cpu.memory[1] = MODE_IMMEDIAT;  // Valid mode
    cpu.memory[2] = 0x33;  // Compare with same value
    TEST_ASSERT_EQUAL_INT(CPU_OK, cpu_step(&cpu));
    TEST_ASSERT_TRUE(cpu.flags & FLAG_ZERO);  // Should be equal
    
    // Test 2: Invalid addressing modes should set error flag
    initCPU(&cpu);
    cpu.PC = 0;
    cpu.memory[0] = OPCODE_LDA;
    cpu.memory[1] = 0xFF;  // Invalid mode (only 0-4 are valid)
    cpu.memory[2] = 0x10;
    TEST_ASSERT_EQUAL_INT(CPU_ERROR, cpu_step(&cpu));
    TEST_ASSERT_TRUE(cpu.flags & FLAG_ERROR);
    
    // Test 3: Invalid branch conditions should not crash (currently no validation)
    initCPU(&cpu);
    cpu.PC = 0;
    cpu.memory[0] = OPCODE_B;
    cpu.memory[1] = 0xFF;  // Invalid condition (only 0-6 are valid)
    cpu.memory[2] = 0x10;
    // This should either handle gracefully or set error flag
    int result3 = cpu_step(&cpu);
    // Current implementation doesn't validate, so it won't branch (default case)
    TEST_ASSERT_EQUAL_INT(CPU_OK, result3);
    TEST_ASSERT_EQUAL_UINT8(3, cpu.PC);  // Should advance normally
    
    // Test 4: Memory wraparound in indexed addressing
    initCPU(&cpu);
    cpu.PC = 0;
    cpu.X = 0x10;
    cpu.memory[0] = OPCODE_LDA;
    cpu.memory[1] = MODE_ABSOLUTE_X;
    cpu.memory[2] = 0xF8;  // 0xF8 + 0x10 = 0x108, should wrap to 0x08
    cpu.memory[0x08] = 0x42;
    TEST_ASSERT_EQUAL_INT(CPU_OK, cpu_step(&cpu));
    TEST_ASSERT_EQUAL_UINT8(0x42, cpu.A);
    
    // Test 5: Indirect addressing edge cases
    initCPU(&cpu);
    cpu.PC = 0;
    cpu.memory[0] = OPCODE_LDA;
    cpu.memory[1] = MODE_INDIRECT;
    cpu.memory[2] = 0x10;
    cpu.memory[0x10] = 0x20;  // Indirect address
    cpu.memory[0x20] = 0x99;  // Final value
    TEST_ASSERT_EQUAL_INT(CPU_OK, cpu_step(&cpu));
    TEST_ASSERT_EQUAL_UINT8(0x99, cpu.A);
    
    // Test 6: Indirect indexed wraparound
    initCPU(&cpu);
    cpu.PC = 0x10;  // Start at higher address to avoid conflicts
    cpu.X = 0x05;
    cpu.memory[0x10] = OPCODE_LDA;
    cpu.memory[0x11] = MODE_INDIRECT_X;
    cpu.memory[0x12] = 0xFB;  // 0xFB + 0x05 = 0x100, wraps to 0x00
    cpu.memory[0x00] = 0x40;  // Indirect address
    cpu.memory[0x40] = 0xAB;  // Final value
    TEST_ASSERT_EQUAL_INT(CPU_OK, cpu_step(&cpu));
    TEST_ASSERT_EQUAL_UINT8(0xAB, cpu.A);
    
    // Test 7: Stack overflow (push when stack full)
    initCPU(&cpu);
    cpu.PC = 0;
    cpu.SP = STACK_BASE - STACK_SIZE;  // Stack full (SP at bottom)
    cpu.A = 0x55;
    cpu.memory[0] = OPCODE_PUSH;
    cpu.memory[1] = 0x00;  // Mode unused for PUSH
    cpu.memory[2] = 0x00;  // Operand unused for PUSH
    TEST_ASSERT_EQUAL_INT(CPU_ERROR, cpu_step(&cpu));
    TEST_ASSERT_TRUE(cpu.flags & FLAG_ERROR);
    
    // Test 8: Stack underflow (pop when stack empty)
    initCPU(&cpu);
    cpu.PC = 0;
    cpu.SP = STACK_BASE;  // Stack empty (SP at top)
    cpu.memory[0] = OPCODE_POP;
    cpu.memory[1] = 0x00;  // Mode unused for POP
    cpu.memory[2] = 0x00;  // Operand unused for POP
    TEST_ASSERT_EQUAL_INT(CPU_ERROR, cpu_step(&cpu));
    TEST_ASSERT_TRUE(cpu.flags & FLAG_ERROR);
    
    // Test 9: PC wraparound at memory boundary
    initCPU(&cpu);
    cpu.PC = 0xFE;  // Near end of memory
    cpu.memory[0xFE] = OPCODE_NOP;
    cpu.memory[0xFF] = 0x00;  // Mode
    cpu.memory[0x00] = 0x00;  // Operand (wraps to beginning)
    TEST_ASSERT_EQUAL_INT(CPU_OK, cpu_step(&cpu));
    TEST_ASSERT_EQUAL_UINT8(0x01, cpu.PC);  // Should wrap to 0x01
    
    // Test 10: Branch to invalid memory location (beyond code space)
    initCPU(&cpu);
    cpu.PC = 0;
    cpu.memory[0] = OPCODE_B;
    cpu.memory[1] = COND_AL;  // Always branch
    cpu.memory[2] = 0xF5;     // Branch to stack area
    TEST_ASSERT_EQUAL_INT(CPU_OK, cpu_step(&cpu));
    TEST_ASSERT_EQUAL_UINT8(0xF5, cpu.PC);
    // Note: This is allowed in current implementation, but might be flagged in FPGA
    
    // Test 11: All flags set simultaneously
    initCPU(&cpu);
    cpu.PC = 0;
    cpu.A = 0x7F;  // Positive number
    cpu.memory[0] = OPCODE_ADD;
    cpu.memory[1] = MODE_IMMEDIAT;
    cpu.memory[2] = 0x01;  // 0x7F + 0x01 = 0x80 (overflow, negative)
    TEST_ASSERT_EQUAL_INT(CPU_OK, cpu_step(&cpu));
    TEST_ASSERT_EQUAL_UINT8(0x80, cpu.A);
    TEST_ASSERT_TRUE(cpu.flags & FLAG_NEGATIVE);
    TEST_ASSERT_TRUE(cpu.flags & FLAG_OVERFLOW);
    TEST_ASSERT_FALSE(cpu.flags & FLAG_ZERO);
    TEST_ASSERT_FALSE(cpu.flags & FLAG_CARRY);
    
    // Test 12: Zero result with carry
    initCPU(&cpu);
    cpu.PC = 0;
    cpu.A = 0xFF;
    cpu.memory[0] = OPCODE_ADD;
    cpu.memory[1] = MODE_IMMEDIAT;
    cpu.memory[2] = 0x01;  // 0xFF + 0x01 = 0x00 with carry
    TEST_ASSERT_EQUAL_INT(CPU_OK, cpu_step(&cpu));
    TEST_ASSERT_EQUAL_UINT8(0x00, cpu.A);
    TEST_ASSERT_TRUE(cpu.flags & FLAG_ZERO);
    TEST_ASSERT_TRUE(cpu.flags & FLAG_CARRY);
    TEST_ASSERT_FALSE(cpu.flags & FLAG_NEGATIVE);
    TEST_ASSERT_FALSE(cpu.flags & FLAG_OVERFLOW);
    
    // Test 13: Subtraction with borrow (no carry)
    initCPU(&cpu);
    cpu.PC = 0;
    cpu.A = 0x50;
    cpu.memory[0] = OPCODE_SUB;
    cpu.memory[1] = MODE_IMMEDIAT;
    cpu.memory[2] = 0x60;  // 0x50 - 0x60 = 0xF0 (borrow occurred)
    TEST_ASSERT_EQUAL_INT(CPU_OK, cpu_step(&cpu));
    TEST_ASSERT_EQUAL_UINT8(0xF0, cpu.A);
    TEST_ASSERT_TRUE(cpu.flags & FLAG_NEGATIVE);
    TEST_ASSERT_FALSE(cpu.flags & FLAG_CARRY);  // Borrow occurred
    TEST_ASSERT_FALSE(cpu.flags & FLAG_ZERO);
    
    // Test 14: STA/STX to stack area (should work but unusual)
    initCPU(&cpu);
    cpu.PC = 0;
    cpu.A = 0x33;
    cpu.memory[0] = OPCODE_STA;
    cpu.memory[1] = MODE_ABSOLUTE;
    cpu.memory[2] = 0xF8;  // Store to stack area
    TEST_ASSERT_EQUAL_INT(CPU_OK, cpu_step(&cpu));
    TEST_ASSERT_EQUAL_UINT8(0x33, cpu.memory[0xF8]);
    
    // Test 15: Multiple stack operations
    initCPU(&cpu);
    cpu.PC = 0;
    cpu.A = 0x11;
    // Push A
    cpu.memory[0] = OPCODE_PUSH;
    cpu.memory[1] = 0x00;
    cpu.memory[2] = 0x00;
    TEST_ASSERT_EQUAL_INT(CPU_OK, cpu_step(&cpu));
    TEST_ASSERT_EQUAL_UINT8(0xFE, cpu.SP);  // SP decremented
    TEST_ASSERT_EQUAL_UINT8(0x11, cpu.memory[0xFF]);  // Value stored
    
    // Change A and pop
    cpu.A = 0x22;
    cpu.PC = 3;
    cpu.memory[3] = OPCODE_POP;
    cpu.memory[4] = 0x00;
    cpu.memory[5] = 0x00;
    TEST_ASSERT_EQUAL_INT(CPU_OK, cpu_step(&cpu));
    TEST_ASSERT_EQUAL_UINT8(0xFF, cpu.SP);  // SP incremented
    TEST_ASSERT_EQUAL_UINT8(0x11, cpu.A);   // Original value restored
}
