#include "../cpu.h"
#include "unity/unity.h"

void ADD_test(void) {
  CPU cpu;
  initCPU(&cpu);
  // ADD #$05 ; A = $10 (16) + $05 (5) => $15 (21)
  cpu.A = 0x10;
  cpu.PC = 0;
  cpu.memory[0] = OPCODE_ADD;
  cpu.memory[1] = MODE_IMMEDIAT;
  cpu.memory[2] = 0x05;
  TEST_ASSERT_EQUAL_INT(CPU_OK, cpu_step(&cpu));
  TEST_ASSERT_EQUAL_UINT8(0x15, cpu.A);
  TEST_ASSERT_FALSE(cpu.flags & FLAG_NEGATIVE);
  TEST_ASSERT_FALSE(cpu.flags & FLAG_OVERFLOW);
  TEST_ASSERT_FALSE(cpu.flags & FLAG_ZERO);
  TEST_ASSERT_FALSE(cpu.flags & FLAG_CARRY);

  // ADD $20 ; A = $10 (16) + memory[$20] ($05/5) => $15 (21)
  cpu.A = 0x10;
  cpu.PC = 10;
  cpu.memory[10] = OPCODE_ADD;
  cpu.memory[11] = MODE_ABSOLUTE;
  cpu.memory[12] = 0x20;
  cpu.memory[0x20] = 0x05;
  TEST_ASSERT_EQUAL_INT(CPU_OK, cpu_step(&cpu));
  TEST_ASSERT_EQUAL_UINT8(0x15, cpu.A);
  TEST_ASSERT_FALSE(cpu.flags & FLAG_NEGATIVE);
  TEST_ASSERT_FALSE(cpu.flags & FLAG_OVERFLOW);
  TEST_ASSERT_FALSE(cpu.flags & FLAG_ZERO);
  TEST_ASSERT_FALSE(cpu.flags & FLAG_CARRY);

  // ADD $30,X ; X = $05 (5), A = $10 (16) + memory[$30+$05] ($05/5) => $15 (21)
  cpu.A = 0x10;
  cpu.PC = 20;
  cpu.X = 0x05;
  cpu.memory[20] = OPCODE_ADD;
  cpu.memory[21] = MODE_ABSOLUTE_X;
  cpu.memory[22] = 0x30;
  cpu.memory[(0x30 + cpu.X) & 0xFF] = 0x05;
  TEST_ASSERT_EQUAL_INT(CPU_OK, cpu_step(&cpu));
  TEST_ASSERT_EQUAL_UINT8(0x15, cpu.A);
  TEST_ASSERT_FALSE(cpu.flags & FLAG_NEGATIVE);
  TEST_ASSERT_FALSE(cpu.flags & FLAG_OVERFLOW);
  TEST_ASSERT_FALSE(cpu.flags & FLAG_ZERO);
  TEST_ASSERT_FALSE(cpu.flags & FLAG_CARRY);

  // ADD ($40) ; memory[$40]=$50, memory[$50]=$05, A = $10 (16) + $05 (5) => $15
  // (21)
  cpu.A = 0x10;
  cpu.PC = 30;
  cpu.memory[30] = OPCODE_ADD;
  cpu.memory[31] = MODE_INDIRECT;
  cpu.memory[32] = 0x40;
  cpu.memory[0x40] = 0x50;
  cpu.memory[0x50] = 0x05;
  TEST_ASSERT_EQUAL_INT(CPU_OK, cpu_step(&cpu));
  TEST_ASSERT_EQUAL_UINT8(0x15, cpu.A);
  TEST_ASSERT_FALSE(cpu.flags & FLAG_NEGATIVE);
  TEST_ASSERT_FALSE(cpu.flags & FLAG_OVERFLOW);
  TEST_ASSERT_FALSE(cpu.flags & FLAG_ZERO);
  TEST_ASSERT_FALSE(cpu.flags & FLAG_CARRY);

  // ADD ($60,X) ; X = $02 (2), memory[$60+$02]=$70, memory[$70]=$05, A = $10
  // (16) + $05 (5) => $15 (21)
  cpu.A = 0x10;
  cpu.PC = 40;
  cpu.X = 0x02;
  cpu.memory[40] = OPCODE_ADD;
  cpu.memory[41] = MODE_INDIRECT_X;
  cpu.memory[42] = 0x60;
  cpu.memory[(0x60 + cpu.X) & 0xFF] = 0x70;
  cpu.memory[0x70] = 0x05;
  TEST_ASSERT_EQUAL_INT(CPU_OK, cpu_step(&cpu));
  TEST_ASSERT_EQUAL_UINT8(0x15, cpu.A);
  TEST_ASSERT_FALSE(cpu.flags & FLAG_NEGATIVE);
  TEST_ASSERT_FALSE(cpu.flags & FLAG_OVERFLOW);
  TEST_ASSERT_FALSE(cpu.flags & FLAG_ZERO);
  TEST_ASSERT_FALSE(cpu.flags & FLAG_CARRY);

  // ADD #$F0 ; A = $10 (16) + $F0 (240) => $00 (0), Carry and Zero set
  cpu.A = 0x10;
  cpu.PC = 50;
  cpu.memory[50] = OPCODE_ADD;
  cpu.memory[51] = MODE_IMMEDIAT;
  cpu.memory[52] = 0xF0;
  TEST_ASSERT_EQUAL_INT(CPU_OK, cpu_step(&cpu));
  TEST_ASSERT_EQUAL_UINT8(0x00, cpu.A);
  TEST_ASSERT_TRUE(cpu.flags & FLAG_CARRY);
  TEST_ASSERT_TRUE(cpu.flags & FLAG_ZERO);
  TEST_ASSERT_FALSE(cpu.flags & FLAG_NEGATIVE);
  TEST_ASSERT_FALSE(cpu.flags & FLAG_OVERFLOW);

  // ADD #$80 ; A = $80 (128) + $80 (128) => $00 (0), Carry, Zero, Overflow set
  cpu.A = 0x80;
  cpu.PC = 60;
  cpu.memory[60] = OPCODE_ADD;
  cpu.memory[61] = MODE_IMMEDIAT;
  cpu.memory[62] = 0x80;
  TEST_ASSERT_EQUAL_INT(CPU_OK, cpu_step(&cpu));
  TEST_ASSERT_EQUAL_UINT8(0x00, cpu.A);
  TEST_ASSERT_TRUE(cpu.flags & FLAG_CARRY);
  TEST_ASSERT_TRUE(cpu.flags & FLAG_ZERO);
  TEST_ASSERT_FALSE(cpu.flags & FLAG_NEGATIVE);
  TEST_ASSERT_TRUE(cpu.flags & FLAG_OVERFLOW);

  // ADD #$FF ; A = $01 (1) + $FF (255) => $00 (0), Carry and Zero set
  cpu.A = 0x01;
  cpu.PC = 70;
  cpu.memory[70] = OPCODE_ADD;
  cpu.memory[71] = MODE_IMMEDIAT;
  cpu.memory[72] = 0xFF;
  TEST_ASSERT_EQUAL_INT(CPU_OK, cpu_step(&cpu));
  TEST_ASSERT_EQUAL_UINT8(0x00, cpu.A);
  TEST_ASSERT_TRUE(cpu.flags & FLAG_CARRY);
  TEST_ASSERT_TRUE(cpu.flags & FLAG_ZERO);
  TEST_ASSERT_FALSE(cpu.flags & FLAG_NEGATIVE);
  TEST_ASSERT_FALSE(cpu.flags & FLAG_OVERFLOW);

  // ADD #$01 ; A = $80 (128) + $01 (1) => $81 (129), Negative set
  cpu.A = 0x80;
  cpu.PC = 80;
  cpu.memory[80] = OPCODE_ADD;
  cpu.memory[81] = MODE_IMMEDIAT;
  cpu.memory[82] = 0x01;
  TEST_ASSERT_EQUAL_INT(CPU_OK, cpu_step(&cpu));
  TEST_ASSERT_EQUAL_UINT8(0x81, cpu.A);
  TEST_ASSERT_FALSE(cpu.flags & FLAG_CARRY);
  TEST_ASSERT_FALSE(cpu.flags & FLAG_ZERO);
  TEST_ASSERT_TRUE(cpu.flags & FLAG_NEGATIVE);
  TEST_ASSERT_FALSE(cpu.flags & FLAG_OVERFLOW);
}
