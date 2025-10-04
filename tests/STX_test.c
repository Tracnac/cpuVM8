#include "../cpu.h"
#include "unity/unity.h"

void STX_test(void) {
  CPU cpu;
  initCPU(&cpu);

  // STX $10 ; X = $CD (205), memory[$10] = $CD (205)
  cpu.X = 0xCD;
  cpu.PC = 0;
  cpu.memory[0] = OPCODE_STX;
  cpu.memory[1] = MODE_ABSOLUTE;
  cpu.memory[2] = 0x10;
  TEST_ASSERT_EQUAL_INT(CPU_OK, cpu_step(&cpu));
  TEST_ASSERT_EQUAL_UINT8(0xCD, cpu.memory[0x10]);

  // STX ($30) ; memory[$30]=$40, X = $CD (205), memory[$40]=$CD (205)
  cpu.X = 0xCD;
  cpu.PC = 20;
  cpu.memory[20] = OPCODE_STX;
  cpu.memory[21] = MODE_INDIRECT;
  cpu.memory[22] = 0x30;
  cpu.memory[0x30] = 0x40;
  TEST_ASSERT_EQUAL_INT(CPU_OK, cpu_step(&cpu));
  TEST_ASSERT_EQUAL_UINT8(0xCD, cpu.memory[0x40]);

}
