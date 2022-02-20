
#ifndef INCLUDE_TIMER_H_
#define INCLUDE_TIMER_H_

#include "types.h"
#include "idt.h"

void init_timer(uint32_t frequency);

void timer_callback(pt_regs *regs);

#endif 	// INCLUDE_TIMER_H_
