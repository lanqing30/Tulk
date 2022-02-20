
#ifndef INCLUDE_SCHEDULER_H_
#define INCLUDE_SCHEDULER_H_

#include "env.h"

// 初始化任务调度
void init_sched();

// 任务调度
void schedule();

// 任务切换
extern void switch_to(struct context_t *prev, struct context_t *next);

#endif 	// INCLUDE_SCHEDULER_H_
