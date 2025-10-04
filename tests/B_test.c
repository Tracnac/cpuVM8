#include "unity/unity.h"
#include "../cpu.h"

void B_test(void) {
    CPU cpu;
    initCPU(&cpu);
    cpu.PC = 0;
    // B AL, $20 ; Unconditional branch, PC = $20 (32)
    cpu.memory[0] = OPCODE_B;
    cpu.memory[1] = COND_AL;
    cpu.memory[2] = 0x20;
    TEST_ASSERT_EQUAL_INT(CPU_OK, cpu_step(&cpu));
    TEST_ASSERT_EQUAL_UINT8(0x20, cpu.PC);

    // B EQ, $30 ; Branch if Z=1, PC = $30 (48)
    cpu.PC = 10;
    cpu.flags |= FLAG_ZERO;
    cpu.memory[10] = OPCODE_B;
    cpu.memory[11] = COND_EQ;
    cpu.memory[12] = 0x30;
    TEST_ASSERT_EQUAL_INT(CPU_OK, cpu_step(&cpu));
    TEST_ASSERT_EQUAL_UINT8(0x30, cpu.PC);

    // B NE, $40 ; Branch if Z=0, PC = $40 (64)
    cpu.PC = 20;
    cpu.flags &= ~FLAG_ZERO;
    cpu.memory[20] = OPCODE_B;
    cpu.memory[21] = COND_NE;
    cpu.memory[22] = 0x40;
    TEST_ASSERT_EQUAL_INT(CPU_OK, cpu_step(&cpu));
    TEST_ASSERT_EQUAL_UINT8(0x40, cpu.PC);

    // B CS, $50 ; Branch if C=1, PC = $50 (80)
    cpu.PC = 30;
    cpu.flags |= FLAG_CARRY;
    cpu.memory[30] = OPCODE_B;
    cpu.memory[31] = COND_CS;
    cpu.memory[32] = 0x50;
    TEST_ASSERT_EQUAL_INT(CPU_OK, cpu_step(&cpu));
    TEST_ASSERT_EQUAL_UINT8(0x50, cpu.PC);

    // B CC, $60 ; Branch if C=0, PC = $60 (96)
    cpu.PC = 40;
    cpu.flags &= ~FLAG_CARRY;
    cpu.memory[40] = OPCODE_B;
    cpu.memory[41] = COND_CC;
    cpu.memory[42] = 0x60;
    TEST_ASSERT_EQUAL_INT(CPU_OK, cpu_step(&cpu));
    TEST_ASSERT_EQUAL_UINT8(0x60, cpu.PC);

    // B MI, $70 ; Branch if N=1, PC = $70 (112)
    cpu.PC = 50;
    cpu.flags |= FLAG_NEGATIVE;
    cpu.memory[50] = OPCODE_B;
    cpu.memory[51] = COND_MI;
    cpu.memory[52] = 0x70;
    TEST_ASSERT_EQUAL_INT(CPU_OK, cpu_step(&cpu));
    TEST_ASSERT_EQUAL_UINT8(0x70, cpu.PC);

    // B PL, $80 ; Branch if N=0, PC = $80 (128)
    cpu.PC = 60;
    cpu.flags &= ~FLAG_NEGATIVE;
    cpu.memory[60] = OPCODE_B;
    cpu.memory[61] = COND_PL;
    cpu.memory[62] = 0x80;
    TEST_ASSERT_EQUAL_INT(CPU_OK, cpu_step(&cpu));
    TEST_ASSERT_EQUAL_UINT8(0x80, cpu.PC);

    // Test branch NOT taken cases - PC should advance by 3
    
    // B EQ, $90 ; Branch if Z=1, but Z=0, so PC should be 73 (70+3)
    cpu.PC = 70;
    cpu.flags &= ~FLAG_ZERO;  // Clear zero flag
    cpu.memory[70] = OPCODE_B;
    cpu.memory[71] = COND_EQ;
    cpu.memory[72] = 0x90;
    TEST_ASSERT_EQUAL_INT(CPU_OK, cpu_step(&cpu));
    TEST_ASSERT_EQUAL_UINT8(73, cpu.PC);  // Should be 70+3

    // B NE, $A0 ; Branch if Z=0, but Z=1, so PC should be 83 (80+3)
    cpu.PC = 80;
    cpu.flags |= FLAG_ZERO;  // Set zero flag
    cpu.memory[80] = OPCODE_B;
    cpu.memory[81] = COND_NE;
    cpu.memory[82] = 0xA0;
    TEST_ASSERT_EQUAL_INT(CPU_OK, cpu_step(&cpu));
    TEST_ASSERT_EQUAL_UINT8(83, cpu.PC);  // Should be 80+3

    // B CS, $B0 ; Branch if C=1, but C=0, so PC should be 93 (90+3)
    cpu.PC = 90;
    cpu.flags &= ~FLAG_CARRY;  // Clear carry flag
    cpu.memory[90] = OPCODE_B;
    cpu.memory[91] = COND_CS;
    cpu.memory[92] = 0xB0;
    TEST_ASSERT_EQUAL_INT(CPU_OK, cpu_step(&cpu));
    TEST_ASSERT_EQUAL_UINT8(93, cpu.PC);  // Should be 90+3

    // B CC, $C0 ; Branch if C=0, but C=1, so PC should be 103 (100+3)
    cpu.PC = 100;
    cpu.flags |= FLAG_CARRY;  // Set carry flag
    cpu.memory[100] = OPCODE_B;
    cpu.memory[101] = COND_CC;
    cpu.memory[102] = 0xC0;
    TEST_ASSERT_EQUAL_INT(CPU_OK, cpu_step(&cpu));
    TEST_ASSERT_EQUAL_UINT8(103, cpu.PC);  // Should be 100+3

    // B MI, $D0 ; Branch if N=1, but N=0, so PC should be 113 (110+3)
    cpu.PC = 110;
    cpu.flags &= ~FLAG_NEGATIVE;  // Clear negative flag
    cpu.memory[110] = OPCODE_B;
    cpu.memory[111] = COND_MI;
    cpu.memory[112] = 0xD0;
    TEST_ASSERT_EQUAL_INT(CPU_OK, cpu_step(&cpu));
    TEST_ASSERT_EQUAL_UINT8(113, cpu.PC);  // Should be 110+3

    // B PL, $E0 ; Branch if N=0, but N=1, so PC should be 123 (120+3)
    cpu.PC = 120;
    cpu.flags |= FLAG_NEGATIVE;  // Set negative flag
    cpu.memory[120] = OPCODE_B;
    cpu.memory[121] = COND_PL;
    cpu.memory[122] = 0xE0;
    TEST_ASSERT_EQUAL_INT(CPU_OK, cpu_step(&cpu));
    TEST_ASSERT_EQUAL_UINT8(123, cpu.PC);  // Should be 120+3
}
