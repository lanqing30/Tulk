#include "sched.h"
#include "debug.h"
#include "env.h"

void init_sched()
{
	// 为当前执行流创建信息结构体 该结构位于当前执行流的栈最低端
	cur_env = (struct Env*) (kern_stack_top - KERN_STACK_SIZE);
	cur_env->next_env = cur_env;
	cur_env->env_id = env_counter;
	env_map[env_counter] = cur_env;
	env_counter ++;
	cur_env->env_parent_id = 0;
	cur_env->env_status = ENV_RUNNABLE;
	cur_env->env_runs = 0;
	cur_env->env_pgdir = pgd_kern;
	cur_env->env_type = ENV_TYPE_KERN;
	cur_env->bg = false;                // 默认的shell进程是前台运行的
}

void schedule() // 这个函数的执行一定发生在中断处理函数之中，也就是说，此时中断是关闭的。
{
        // 进程不会调度阻塞的进程，
        // 为什么会阻塞？后台进程如果进行IO的系统调用，就会阻塞，如果被放到前台那么就不回发生阻塞。
        // 如果有前台进程正在运行，那么不调度内核进程（默认是env_map[0]）
        struct Env* tail = cur_env->next_env;
        struct Env* pre = cur_env;
        while (tail != cur_env) {
            if (tail->env_status == ENV_RUNNABLE) {
                if (tail->env_id == 0 && has_front_env) { // 如果有前台进程正在运行，那么不调度内核进程（默认是env_map[0]）
                    pre = tail;
                    tail = tail->next_env;
                }
                else break;
            } else if (tail->env_status == ENV_ZOMBIE) {
                struct Env* tmp = tail;
                pre->next_env = tail->next_env;
                tail = pre->next_env;
                env_free(tmp);
            } else {
                pre = tail;
                tail = tail->next_env;
            }
        }
        if (tail == cur_env) return;

        struct Env* next = tail;
		struct Env* prev = cur_env;
		cur_env = next;
		switch_to(&(prev->context), &(cur_env->context));

}




