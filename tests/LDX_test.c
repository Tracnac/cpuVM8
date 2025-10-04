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

  // LDX ($40) ; memory[$40]=$50, memory[$50]=$77 (119), X = $77 (119)
  cpu.flags = 0;
  cpu.PC = 20;
  cpu.memory[20] = OPCODE_LDX;
  cpu.memory[21] = MODE_INDIRECT;
  cpu.memory[22] = 0x40;
  cpu.memory[0x40] = 0x50;
  cpu.memory[0x50] = 0x77;
  TEST_ASSERT_EQUAL_INT(CPU_OK, cpu_step(&cpu));
  TEST_ASSERT_EQUAL_UINT8(0x77, cpu.X);
  TEST_ASSERT_FALSE(cpu.flags & FLAG_ZERO);
  TEST_ASSERT_FALSE(cpu.flags & FLAG_NEGATIVE);


}
