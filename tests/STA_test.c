#include "../cpu.h"
#include "unity/unity.h"

void STA_test(void) {
  CPU cpu;
  initCPU(&cpu);
  cpu.A = 0xAB;

  // STA $10 ; A = $AB (171), memory[$10] = $AB (171)
  cpu.PC = 0;
  cpu.memory[0] = OPCODE_STA;
  cpu.memory[1] = MODE_ABSOLUTE;
  cpu.memory[2] = 0x10;
  TEST_ASSERT_EQUAL_INT(CPU_OK, cpu_step(&cpu));
  TEST_ASSERT_EQUAL_UINT8(0xAB, cpu.memory[0x10]);

  // STA $20,X ; X = $05 (5), A = $AB (171), memory[$20+$05] = $AB (171)
  cpu.PC = 10;
  cpu.X = 0x05;
  cpu.memory[10] = OPCODE_STA;
  cpu.memory[11] = MODE_INDEXED_X;
  cpu.memory[12] = 0x20;
  TEST_ASSERT_EQUAL_INT(CPU_OK, cpu_step(&cpu));
  TEST_ASSERT_EQUAL_UINT8(0xAB, cpu.memory[(0x20 + cpu.X) & 0xFF]);

  // STA ($30) ; memory[$30]=$40, A = $AB (171), memory[$40]=$AB (171)
  cpu.PC = 20;
  cpu.memory[20] = OPCODE_STA;
  cpu.memory[21] = MODE_INDIRECT;
  cpu.memory[22] = 0x30;
  cpu.memory[0x30] = 0x40;
  TEST_ASSERT_EQUAL_INT(CPU_OK, cpu_step(&cpu));
  TEST_ASSERT_EQUAL_UINT8(0xAB, cpu.memory[0x40]);

  // STA ($50,X) ; X = $02 (2), memory[$50+$02]=$60, A = $AB (171),
  // memory[$60]=$AB (171)
  cpu.PC = 30;
  cpu.X = 0x02;
  cpu.memory[30] = OPCODE_STA;
  cpu.memory[31] = MODE_INDIRECT_INDEXED_X;
  cpu.memory[32] = 0x50;
  cpu.memory[(0x50 + cpu.X) & 0xFF] = 0x60;
  TEST_ASSERT_EQUAL_INT(CPU_OK, cpu_step(&cpu));
  TEST_ASSERT_EQUAL_UINT8(0xAB, cpu.memory[0x60]);
}
