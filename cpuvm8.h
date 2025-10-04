#ifndef CPUVM8_H
#define CPUVM8_H

#include "cpu.h"
#include <stdio.h>      // printf...
#include <time.h>       // clock_gettime...
#include <unistd.h>     // usleep...

#define INSTR_COUNT 10000000

void dump_cpu(const CPU *cpu);
#endif
