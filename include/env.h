
#ifndef INCLUDE_ENV_H_
#define INCLUDE_ENV_H_
#include "common.h"
#include "types.h"
#include "elf.h"
#define MAX_ENV_NUM 1024 // 最多开1024个进程


// Values of env_status in struct Env
enum {
	ENV_ZOMBIE = 0,
	ENV_RUNNABLE = 1,
	ENV_RUNNING = 2,    // 一般来说，在多核CPU时会用到
	ENV_STOPED = 3,
	ENV_BLOCKED = 4
};

// Special environment types
enum EnvType {
	ENV_TYPE_USER = 0,
    ENV_TYPE_KERN = 1
};

// blow is from JOS
struct PushRegs {
	/* registers as pushed by pusha */
	uint32_t reg_edi;
	uint32_t reg_esi;
	uint32_t reg_ebp;
	uint32_t reg_oesp;		/* Useless */
	uint32_t reg_ebx;
	uint32_t reg_edx;
	uint32_t reg_ecx;
	uint32_t reg_eax;
} __attribute__((packed));


// 内核进程的上下文切换保存的信息
struct context_t {
    uint32_t esp;
    uint32_t ebp;
    uint32_t ebx;
    uint32_t esi;
    uint32_t edi;
    uint32_t eflags;
};

struct Env {
    struct context_t context;           // 进程切换需要的上下文信息
	struct Env *next_env;
	envid_t env_id;			            // Unique environment identifier
	envid_t env_parent_id;		        // env_id of this env's parent
	enum EnvType env_type;		        // Indicates special system environments
	volatile unsigned env_status;		// Status of the environment
	uint32_t env_runs;		            // Number of times environment has run
	// Address space
	pde_t *env_pgdir;		            // 用户进程
	bool bg;                            // 当前是不是后台运行
};


extern uint16_t env_counter;
extern struct Env envs[];
extern struct Env* env_map[]; // 如何通过进程的id找到一个进程的指针
extern struct Env* free_env_stack[];
extern struct Env* cur_env;
extern uint32_t kern_env_stack[];
extern bool has_front_env;

void env_free(struct Env* e);
void init_env();
int env_alloc(struct Env **newenv_store, envid_t parent_id);
void load_icode(const char* module_name, void* argv, bool bg);

#endif
