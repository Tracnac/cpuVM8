#include "../cpu.h"
#include "unity/unity.h"

void LDX_test(void) {
  CPU cpu;
  initCPU(&cpu);

  // LDX #$42 ; X = #$42 (66)
  cpu.PC = 0;
  cpu.memory[0] = OPCODE_LDX;
  cpu.memory[1] = MODE_IMMEDIAT;
  cpu.memory[2] = 0x42;
  TEST_ASSERT_EQUAL_INT(CPU_OK, cpu_step(&cpu));
  TEST_ASSERT_EQUAL_UINT8(0x42, cpu.X);
  TEST_ASSERT_FALSE(cpu.flags & FLAG_ZERO);
  TEST_ASSERT_FALSE(cpu.flags & FLAG_NEGATIVE);

  // LDX #$00 ; X = #$00 (0), should set Zero flag
  cpu.flags = 0;
  cpu.PC = 50;
  cpu.memory[50] = OPCODE_LDX;
  cpu.memory[51] = MODE_IMMEDIAT;
  cpu.memory[52] = 0x00;
  TEST_ASSERT_EQUAL_INT(CPU_OK, cpu_step(&cpu));
  TEST_ASSERT_EQUAL_UINT8(0x00, cpu.X);
  TEST_ASSERT_TRUE(cpu.flags & FLAG_ZERO);
  TEST_ASSERT_FALSE(cpu.flags & FLAG_NEGATIVE);

  // LDX $20 ; memory[$20] = $55 (85), X = $55 (85)
  cpu.flags = 0;
  cpu.PC = 10;
  cpu.memory[10] = OPCODE_LDX;
  cpu.memory[11] = MODE_ABSOLUTE;
  cpu.memory[12] = 0x20;
  cpu.memory[0x20] = 0x55;
  TEST_ASSERT_EQUAL_INT(CPU_OK, cpu_step(&cpu));
  TEST_ASSERT_EQUAL_UINT8(0x55, cpu.X);
  TEST_ASSERT_FALSE(cpu.flags & FLAG_ZERO);
  TEST_ASSERT_FALSE(cpu.flags & FLAG_NEGATIVE);

  // LDX $30,X ; X = $05 (5), memory[$30+$05] = $66 (102), X = $66 (102)
  cpu.flags = 0;
  cpu.PC = 20;
  cpu.X = 0x05;
  cpu.memory[20] = OPCODE_LDX;
  cpu.memory[21] = MODE_INDEXED_X;
  cpu.memory[22] = 0x30;
  cpu.memory[(0x30 + cpu.X) & 0xFF] = 0x66;
  TEST_ASSERT_EQUAL_INT(CPU_OK, cpu_step(&cpu));
  TEST_ASSERT_EQUAL_UINT8(0x66, cpu.X);
  TEST_ASSERT_FALSE(cpu.flags & FLAG_ZERO);
  TEST_ASSERT_FALSE(cpu.flags & FLAG_NEGATIVE);

  // LDX ($40) ; memory[$40]=$50, memory[$50]=$77 (119), X = $77 (119)
  cpu.flags = 0;
  cpu.PC = 30;
  cpu.memory[30] = OPCODE_LDX;
  cpu.memory[31] = MODE_INDIRECT;
  cpu.memory[32] = 0x40;
  cpu.memory[0x40] = 0x50;
  cpu.memory[0x50] = 0x77;
  TEST_ASSERT_EQUAL_INT(CPU_OK, cpu_step(&cpu));
  TEST_ASSERT_EQUAL_UINT8(0x77, cpu.X);
  TEST_ASSERT_FALSE(cpu.flags & FLAG_ZERO);
  TEST_ASSERT_FALSE(cpu.flags & FLAG_NEGATIVE);

  // LDX ($60,X) ; X = $02 (2), memory[$60+$02]=$70, memory[$70]=$88 (136), X =
  // $88 (136)
  cpu.flags = 0;
  cpu.PC = 40;
  cpu.X = 0x02;
  cpu.memory[40] = OPCODE_LDX;
  cpu.memory[41] = MODE_INDIRECT_INDEXED_X;
  cpu.memory[42] = 0x60;
  cpu.memory[(0x60 + cpu.X) & 0xFF] = 0x70;
  cpu.memory[0x70] = 0x88;
  TEST_ASSERT_EQUAL_INT(CPU_OK, cpu_step(&cpu));
  TEST_ASSERT_EQUAL_UINT8(0x88, cpu.X);
  TEST_ASSERT_FALSE(cpu.flags & FLAG_ZERO);
  TEST_ASSERT_TRUE(cpu.flags & FLAG_NEGATIVE);
}
