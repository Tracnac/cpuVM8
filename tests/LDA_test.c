#include "../cpu.h"
#include "unity/unity.h"

void LDA_test(void) {
  CPU cpu;
  initCPU(&cpu);

  // LDA #$42 ; A = #$42 (66)
  cpu.PC = 0;
  cpu.memory[0] = OPCODE_LDA;
  cpu.memory[1] = MODE_IMMEDIAT;
  cpu.memory[2] = 0x42;
  TEST_ASSERT_EQUAL_INT(CPU_OK, cpu_step(&cpu));
  TEST_ASSERT_EQUAL_UINT8(0x42, cpu.A);
  TEST_ASSERT_FALSE(cpu.flags & FLAG_ZERO);
  TEST_ASSERT_FALSE(cpu.flags & FLAG_NEGATIVE);

  // LDA $20 ; memory[$20] = $55 (85), A = $55 (85)
  cpu.flags = 0;
  cpu.PC = 10;
  cpu.memory[10] = OPCODE_LDA;
  cpu.memory[11] = MODE_ABSOLUTE;
  cpu.memory[12] = 0x20;
  cpu.memory[0x20] = 0x55;
  TEST_ASSERT_EQUAL_INT(CPU_OK, cpu_step(&cpu));
  TEST_ASSERT_EQUAL_UINT8(0x55, cpu.A);
  TEST_ASSERT_FALSE(cpu.flags & FLAG_ZERO);
  TEST_ASSERT_FALSE(cpu.flags & FLAG_NEGATIVE);

  // LDA $30,X ; X = $05 (5), memory[$30+$05] = $66 (102), A = $66 (102)
  cpu.flags = 0;
  cpu.PC = 20;
  cpu.X = 0x05;
  cpu.memory[20] = OPCODE_LDA;
  cpu.memory[21] = MODE_ABSOLUTE_X;
  cpu.memory[22] = 0x30;
  cpu.memory[(0x30 + cpu.X) & 0xFF] = 0x66;
  TEST_ASSERT_EQUAL_INT(CPU_OK, cpu_step(&cpu));
  TEST_ASSERT_EQUAL_UINT8(0x66, cpu.A);
  TEST_ASSERT_FALSE(cpu.flags & FLAG_ZERO);
  TEST_ASSERT_FALSE(cpu.flags & FLAG_NEGATIVE);

  // LDA ($40) ; memory[$40]=$50, memory[$50]=$77 (119), A = $77 (119)
  cpu.flags = 0;
  cpu.PC = 30;
  cpu.memory[30] = OPCODE_LDA;
  cpu.memory[31] = MODE_INDIRECT;
  cpu.memory[32] = 0x40;
  cpu.memory[0x40] = 0x50;
  cpu.memory[0x50] = 0x77;
  TEST_ASSERT_EQUAL_INT(CPU_OK, cpu_step(&cpu));
  TEST_ASSERT_EQUAL_UINT8(0x77, cpu.A);
  TEST_ASSERT_FALSE(cpu.flags & FLAG_ZERO);
  TEST_ASSERT_FALSE(cpu.flags & FLAG_NEGATIVE);

  // LDA ($60,X) ; X = $02 (2), memory[$60+$02]=$70, memory[$70]=$88 (136), A =
  // $88 (136)
  cpu.flags = 0;
  cpu.PC = 40;
  cpu.X = 0x02;
  cpu.memory[40] = OPCODE_LDA;
  cpu.memory[41] = MODE_INDIRECT_X;
  cpu.memory[42] = 0x60;
  cpu.memory[(0x60 + cpu.X) & 0xFF] = 0x70;
  cpu.memory[0x70] = 0x88;
  TEST_ASSERT_EQUAL_INT(CPU_OK, cpu_step(&cpu));
  TEST_ASSERT_EQUAL_UINT8(0x88, cpu.A);
  TEST_ASSERT_FALSE(cpu.flags & FLAG_ZERO);
  TEST_ASSERT_TRUE(cpu.flags & FLAG_NEGATIVE);

  // LDA #$00 ; A = #$00 (0), should set Zero flag
  cpu.flags = 0;
  cpu.PC = 50;
  cpu.memory[50] = OPCODE_LDA;
  cpu.memory[51] = MODE_IMMEDIAT;
  cpu.memory[52] = 0x00;
  TEST_ASSERT_EQUAL_INT(CPU_OK, cpu_step(&cpu));
  TEST_ASSERT_EQUAL_UINT8(0x00, cpu.A);
  TEST_ASSERT_TRUE(cpu.flags & FLAG_ZERO);
  TEST_ASSERT_FALSE(cpu.flags & FLAG_NEGATIVE);
}
