#ifndef __NEMU_H__
#define __NEMU_H__

#include "common.h"
#include "memory/memory.h"
#include "isa/reg.h"

extern CPU_state cpu;
void reg_test();
void isa_reg_display();
uint32_t isa_reg_str2val(const char *s, bool *success);

#endif
