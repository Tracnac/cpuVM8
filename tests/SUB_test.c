#include "unity/unity.h"
#include "../cpu.h"

void SUB_test(void) {
    CPU cpu;
    initCPU(&cpu);

    // SUB #$05 ; A = $10 (16) - $05 (5) => $0B (11)
    cpu.A = 0x10;
    cpu.PC = 0;
    cpu.memory[0] = OPCODE_SUB;
    cpu.memory[1] = MODE_IMMEDIAT;
    cpu.memory[2] = 0x05;
    TEST_ASSERT_EQUAL_INT(CPU_OK, cpu_step(&cpu));
    TEST_ASSERT_EQUAL_UINT8(0x0B, cpu.A);
    TEST_ASSERT_FALSE(cpu.flags & FLAG_ZERO);
    TEST_ASSERT_FALSE(cpu.flags & FLAG_NEGATIVE);
    TEST_ASSERT_TRUE(cpu.flags & FLAG_CARRY);
    TEST_ASSERT_FALSE(cpu.flags & FLAG_OVERFLOW);

    // SUB $20 ; memory[$20] = $05 (5), A = $10 (16) - $05 (5) => $0B (11)
    cpu.A = 0x10;
    cpu.flags = 0;
    cpu.PC = 10;
    cpu.memory[10] = OPCODE_SUB;
    cpu.memory[11] = MODE_ABSOLUTE;
    cpu.memory[12] = 0x20;
    cpu.memory[0x20] = 0x05;
    TEST_ASSERT_EQUAL_INT(CPU_OK, cpu_step(&cpu));
    TEST_ASSERT_EQUAL_UINT8(0x0B, cpu.A);
    TEST_ASSERT_FALSE(cpu.flags & FLAG_ZERO);
    TEST_ASSERT_FALSE(cpu.flags & FLAG_NEGATIVE);
    TEST_ASSERT_TRUE(cpu.flags & FLAG_CARRY);
    TEST_ASSERT_FALSE(cpu.flags & FLAG_OVERFLOW);

    // SUB $30,X ; X = $05 (5), memory[$30+$05] = $05 (5), A = $10 (16) - $05 (5) => $0B (11)
    cpu.A = 0x10;
    cpu.flags = 0;
    cpu.PC = 20;
    cpu.X = 0x05;
    cpu.memory[20] = OPCODE_SUB;
    cpu.memory[21] = MODE_INDEXED_X;
    cpu.memory[22] = 0x30;
    cpu.memory[(0x30 + cpu.X) & 0xFF] = 0x05;
    TEST_ASSERT_EQUAL_INT(CPU_OK, cpu_step(&cpu));
    TEST_ASSERT_EQUAL_UINT8(0x0B, cpu.A);
    TEST_ASSERT_FALSE(cpu.flags & FLAG_ZERO);
    TEST_ASSERT_FALSE(cpu.flags & FLAG_NEGATIVE);
    TEST_ASSERT_TRUE(cpu.flags & FLAG_CARRY);
    TEST_ASSERT_FALSE(cpu.flags & FLAG_OVERFLOW);

    // SUB ($40) ; memory[$40]=$50, memory[$50]=$05 (5), A = $10 (16) - $05 (5) => $0B (11)
    cpu.A = 0x10;
    cpu.flags = 0;
    cpu.PC = 30;
    cpu.memory[30] = OPCODE_SUB;
    cpu.memory[31] = MODE_INDIRECT;
    cpu.memory[32] = 0x40;
    cpu.memory[0x40] = 0x50;
    cpu.memory[0x50] = 0x05;
    TEST_ASSERT_EQUAL_INT(CPU_OK, cpu_step(&cpu));
    TEST_ASSERT_EQUAL_UINT8(0x0B, cpu.A);
    TEST_ASSERT_FALSE(cpu.flags & FLAG_ZERO);
    TEST_ASSERT_FALSE(cpu.flags & FLAG_NEGATIVE);
    TEST_ASSERT_TRUE(cpu.flags & FLAG_CARRY);
    TEST_ASSERT_FALSE(cpu.flags & FLAG_OVERFLOW);

    // SUB ($60,X) ; X = $02 (2), memory[$60+$02]=$70, memory[$70]=$05 (5), A = $10 (16) - $05 (5) => $0B (11)
    cpu.A = 0x10;
    cpu.flags = 0;
    cpu.PC = 40;
    cpu.X = 0x02;
    cpu.memory[40] = OPCODE_SUB;
    cpu.memory[41] = MODE_INDIRECT_INDEXED_X;
    cpu.memory[42] = 0x60;
    cpu.memory[(0x60 + cpu.X) & 0xFF] = 0x70;
    cpu.memory[0x70] = 0x05;
    TEST_ASSERT_EQUAL_INT(CPU_OK, cpu_step(&cpu));
    TEST_ASSERT_EQUAL_UINT8(0x0B, cpu.A);
    TEST_ASSERT_FALSE(cpu.flags & FLAG_ZERO);
    TEST_ASSERT_FALSE(cpu.flags & FLAG_NEGATIVE);
    TEST_ASSERT_TRUE(cpu.flags & FLAG_CARRY);
    TEST_ASSERT_FALSE(cpu.flags & FLAG_OVERFLOW);
}
