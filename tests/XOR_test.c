#include "unity/unity.h"
#include "../cpu.h"

void XOR_test(void) {
    CPU cpu;
    initCPU(&cpu);

    // XOR #$0F ; A = $F0 (240) ^ $0F (15) => $FF (255)
    cpu.A = 0xF0;
    cpu.PC = 0;
    cpu.memory[0] = OPCODE_XOR;
    cpu.memory[1] = MODE_IMMEDIAT;
    cpu.memory[2] = 0x0F;
    TEST_ASSERT_EQUAL_INT(CPU_OK, cpu_step(&cpu));
    TEST_ASSERT_EQUAL_UINT8(0xFF, cpu.A);
    TEST_ASSERT_FALSE(cpu.flags & FLAG_ZERO);
    TEST_ASSERT_TRUE(cpu.flags & FLAG_NEGATIVE);

    // XOR $20 ; memory[$20] = $0F (15), A = $F0 (240) ^ $0F (15) => $FF (255)
    cpu.A = 0xF0;
    cpu.flags = 0;
    cpu.PC = 10;
    cpu.memory[10] = OPCODE_XOR;
    cpu.memory[11] = MODE_ABSOLUTE;
    cpu.memory[12] = 0x20;
    cpu.memory[0x20] = 0x0F;
    TEST_ASSERT_EQUAL_INT(CPU_OK, cpu_step(&cpu));
    TEST_ASSERT_EQUAL_UINT8(0xFF, cpu.A);
    TEST_ASSERT_FALSE(cpu.flags & FLAG_ZERO);
    TEST_ASSERT_TRUE(cpu.flags & FLAG_NEGATIVE);

    // XOR $30,X ; X = $05 (5), memory[$30+$05] = $0F (15), A = $F0 (240) ^ $0F (15) => $FF (255)
    cpu.A = 0xF0;
    cpu.flags = 0;
    cpu.PC = 20;
    cpu.X = 0x05;
    cpu.memory[20] = OPCODE_XOR;
    cpu.memory[21] = MODE_ABSOLUTE_X;
    cpu.memory[22] = 0x30;
    cpu.memory[(0x30 + cpu.X) & 0xFF] = 0x0F;
    TEST_ASSERT_EQUAL_INT(CPU_OK, cpu_step(&cpu));
    TEST_ASSERT_EQUAL_UINT8(0xFF, cpu.A);
    TEST_ASSERT_FALSE(cpu.flags & FLAG_ZERO);
    TEST_ASSERT_TRUE(cpu.flags & FLAG_NEGATIVE);

    // XOR ($40) ; memory[$40]=$50, memory[$50]=$0F (15), A = $F0 (240) ^ $0F (15) => $FF (255)
    cpu.A = 0xF0;
    cpu.flags = 0;
    cpu.PC = 30;
    cpu.memory[30] = OPCODE_XOR;
    cpu.memory[31] = MODE_INDIRECT;
    cpu.memory[32] = 0x40;
    cpu.memory[0x40] = 0x50;
    cpu.memory[0x50] = 0x0F;
    TEST_ASSERT_EQUAL_INT(CPU_OK, cpu_step(&cpu));
    TEST_ASSERT_EQUAL_UINT8(0xFF, cpu.A);
    TEST_ASSERT_FALSE(cpu.flags & FLAG_ZERO);
    TEST_ASSERT_TRUE(cpu.flags & FLAG_NEGATIVE);

    // XOR ($60,X) ; X = $02 (2), memory[$60+$02]=$70, memory[$70]=$0F (15), A = $F0 (240) ^ $0F (15) => $FF (255)
    cpu.A = 0xF0;
    cpu.flags = 0;
    cpu.PC = 40;
    cpu.X = 0x02;
    cpu.memory[40] = OPCODE_XOR;
    cpu.memory[41] = MODE_INDIRECT_X;
    cpu.memory[42] = 0x60;
    cpu.memory[(0x60 + cpu.X) & 0xFF] = 0x70;
    cpu.memory[0x70] = 0x0F;
    TEST_ASSERT_EQUAL_INT(CPU_OK, cpu_step(&cpu));
    TEST_ASSERT_EQUAL_UINT8(0xFF, cpu.A);
    TEST_ASSERT_FALSE(cpu.flags & FLAG_ZERO);
    TEST_ASSERT_TRUE(cpu.flags & FLAG_NEGATIVE);
}
