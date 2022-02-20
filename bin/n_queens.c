#include "types.h"
#include "syscall.h"
#include "debug.h"

MODULE("n_queen") void queenentry() { queenmain(); _exit(); }


int C[20];
bool vis[3][40];
int tot=0;
int n=8;

MODULE("n_queen") void search2(int cur)
{
    int i=0;
    if(cur == n) tot++;
    else for(i=0;i<n;i++){
            if(!vis[0][i]&&!vis[1][cur+i]&&!vis[2][cur-i+n]){
                vis[1][cur+i]=vis[2][cur-i+n]=vis[0][i]=1;
                search2(cur+1);
                vis[1][cur+i]=vis[0][i]=vis[2][cur-i+n]=0;
            }
    }
}

MODULE("n_queen") int queenmain() {
	search2(0);
	// 实际上这里不应使用printk的
    	printk("8 queens problem solved : %d\n", tot);
	return 0;
}

