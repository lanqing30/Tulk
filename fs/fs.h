#include "global.h"


#define DISK_MAP (DISK_SUPER + (1<<12))
#define DISK_INODE (DISK_SUPER + (1<<12) * 3)
#define DISK_END (DISK_SUPER + (1<<28))

#define MAX_DEPTH 20
#define MAX_PATH_LENTH 256

#define READ 0x00000001
#define WRITE 0x00000002
#define EXE 0x00000004
#define FILE_NAME_MAXN 200

typedef
struct File { // 256 Bytes

    char filename[FILE_NAME_MAXN];
    uint32_t filesize;
    uint32_t is_dir;
    uint32_t permission;
    uint32_t inodes[10]; // 40Bytes
    uint32_t idirect;

} File;

typedef
struct Fd {

    File* file;
    void* pointer; // 4bytes

} Fd;


extern char* pwd() ;

extern int cd(const char* foldername) ;
extern int mkdir(const char* foldername) ;

extern int creat(const char* filename);

extern void cat(const char* filename) ;

extern void ls() ;

extern void init_map();

extern int creatfile(File* dir, const char* name,  uint32_t perm, int is_folder);

extern int writefile(const char* filename, const char* buffer, uint32_t len);


