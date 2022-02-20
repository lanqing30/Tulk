#ifndef INCLUDE_destroy_H
#define INCLUDE_destroy_H
#include "env.h"

#include "console.h"

enum {

    SHELL_JOBS = 0,
    SHELL_FG,
    SHELL_RERUN,
    SHELL_KILL,
    SHELL_STOP,
    SHELL_HELP,
    SHELL_EXE

};

// TOOD，应该优化&统一模式
typedef struct cmd_t {
    uint8_t cmd_type;
    uint8_t bg;
    pid_t tar_pid;
    char exe_name[1<<4];
    char args[1<<3][1<<4];      // 尽量对齐
    uint8_t argcnt;
} __attribute__((packed)) cmd_t;

// 全局使用的记录当前命令行的信息
extern cmd_t runinfo;

void listing_all_jobs();

int32_t parse_num(char* s);

void show_help();

int8_t check_pid(pid_t pid);

void parse_cmd(const char* s);

void listing_all_jobs();

#endif
