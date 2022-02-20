#include "env.h"
#include "console.h"
#include "string.h"
#include "shell.h"
#include "debug.h"
/**
    tiny-shell暂时支持如下命令：

    在没有前台进程运行的情况下，可以执行如下命令

    jobs: 列出后台进程的状态：（正在运行、IO阻塞、暂停、僵尸进程）
    fg x: 将pid为x的进程放到前台运行
    kill x：杀死pid为x的进程
    bg x: 将暂停的进程放到后台运行
    exename arg1 arg2 运行参数为arg1 arg2的进程，加 & 为后台运行 否则为前台运行

    在有前台进程运行的情况下，可以执行如下命令

    按下ctrl-B 将进程放到后台运行

*/

cmd_t runinfo;
#define ISDIGIT(x) (('0' <= x) &&(x <= '9'))
#define GETNUM(x) (x-'0')

// 从s开始的位置，往后解析第一个数字
int32_t parse_num(char* s) {
    int32_t ret = -1;
    while (s && *s == ' ') s++; // 跳过空格
    if (!ISDIGIT(*s)) return ret;
    ret = 0;
    while(s && ISDIGIT(*s)) {
        ret = ret * 10 + GETNUM(*s);
        s ++;
    }
    return ret;
}

bool parse_args(char *p) {

    // 解析字符串中的参数，最后一个参数是空字符串结尾
    char *s = p;
    char* b;
    for (;;) {
        while (*s == ' ') s++; if (*s == '\0') break;
        b = s; // 设置开头的字符
        while (*s != ' ') s++;
        if (*s == '\0') { // 结尾了
            strcpy(runinfo.args[runinfo.argcnt++], b);
            break;
        } else {
            *s++ = '\0';
            strcpy(runinfo.args[runinfo.argcnt++], b);
        }
    }

    if (strcmp(runinfo.args[runinfo.argcnt-1], "&") == 0) {
        strcpy(runinfo.args[runinfo.argcnt-1], "");
        runinfo.argcnt--;
        return 1;
    } else {
        return 0;
    }

}



void parse_cmd(const char* s) {
    memset(&runinfo, 0, sizeof(runinfo));
    char* p = s;
    // 跳过前面的字符串
    while(p && *p == ' ') p++;
    char *b = p;
    while (p && *p != ' ') p++;
    *p = '\0';
    if (strcmp(b, "jobs") == 0) {
        runinfo.cmd_type = SHELL_JOBS;
    } else if (strcmp(b, "fg") == 0) {
        runinfo.cmd_type = SHELL_FG;
        runinfo.tar_pid = parse_num(++p);
    } else if (strcmp(b, "bring") == 0) {
        runinfo.cmd_type = SHELL_RERUN;
        runinfo.tar_pid = parse_num(++p);
    } else if (strcmp(b, "kill") == 0) {
        runinfo.cmd_type = SHELL_KILL;
        runinfo.tar_pid = parse_num(++p);
    } else if (strcmp(b, "stop") == 0) {
        runinfo.cmd_type = SHELL_STOP;
        runinfo.tar_pid = parse_num(++p);
    } else if (strcmp(b, "help") == 0) {
        runinfo.cmd_type = SHELL_HELP;
    } else {
        runinfo.cmd_type = SHELL_EXE;               // 暂时没有参数并且是运行在后台的
        memcpy(runinfo.args[runinfo.argcnt++], b, strlen(b) + 1);
        runinfo.exe_name[0] = '.';
        memcpy(runinfo.exe_name+1, b, strlen(b) + 1);
        strcat(runinfo.exe_name, ".text");
        runinfo.bg = parse_args(++p);
    }

}


void listing_all_jobs() {

    struct Env* tail = cur_env->next_env;
    while (tail != cur_env) {
        printk("[%04d]  ", tail->env_id);
        switch(tail->env_status) {
            case ENV_RUNNABLE:
                printk("Runnig  ");
                break;
            case ENV_BLOCKED:
                printk("Blocked  ");
                break;
            case ENV_STOPED:
                printk("Stoped  ");
                break;
            case ENV_ZOMBIE:
                printk("Zombie  ");
                break;
            default:
                printk("Unknown  ");
                break;
        }
        console_putc_color('\n', rc_black, rc_white);
        tail = tail->next_env;
    }
}


int8_t check_pid(pid_t pid) {
    if (pid < 0 || pid > MAX_ENV_NUM) return 0;
    if (!env_map[pid]) return 0;
    return 1;
}


void show_help() {
    printk("pr [&]         -- launch a program called \"pr\" (foreground by default).\n");
    printk("fg pid         -- bring a program from back ground to foreground.\n");
    printk("kill pid       -- kill a program.\n");
    printk("stop pid       -- stop a program.\n");
    printk("bring pid      -- bring a stopped program running.\n");
    printk("jobs           -- show all the alive program.\n");
    printk("help           -- instructions.\n");
}
