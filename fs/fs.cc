#include "fs.h"

static void* DISK_SUPER;
static char* search_begin;
static File* where[MAX_DEPTH];
static int depth;
static pathbuffer[MAX_PATH_LENTH];

static void* getinodeaddress(uint32_t inode) {
    return (void*) (DISK_INODE + (1<<12) * inode);
}

static File* getfilefromfolderbyno(File* dir, int no) {
    if (no < 160) {
        int idx = no / 16;
        int offset = no % 16;
        return getinodeaddress(dir->inodes[idx]) + 256 * offset;
    } else {
        no = no - 160;
        int idx = no / 16;
        int offset = no % 16;
        uint32_t* indir = getinodeaddress(dir->idirect);
        indir += idx;
        return getinodeaddress(*indir) + 256 * offset;
    }
}

static File* getfilefromfolderbyname(File* folder, const char* filename) {
    assert(folder->is_dir == 1, "Argument folder is not a directory");
    int i;
    for (i=0; i<folder->filesize; i++) {
        File* temp = getfilefromfolderbyno(folder, i);
        if (strcmp(filename, temp->filename)==0)
            return temp;
    }
    return NULL;
}

static void display(char* begin, int size) {
    char* c = begin;
    while(size--) putchar(*c++);
}

static uint32_t transform(char* emptyPointer, int k) {
    return (emptyPointer - (char*)DISK_MAP) * 8 + k;
}

static uint32_t find_empty_inode() {

    char* search_end = (char*)(search_begin == (char*)DISK_MAP) ? (DISK_INODE - 1) : (search_begin - 1);
    char* c = search_begin;
    int k = 0;
    while (c != search_end) {
        // check each bit of the byte
        for (k=0; k<8; k++) {
            if(!((*c) & (1<<k))) {
                uint32_t ans = transform(c, k);
                search_begin = c;
                *c = *c | (1<<k);
                return ans;
            }
        }
        c += 1;
        if (c == (char*)(DISK_END)) c = (char*)DISK_MAP;
    }
// simple method but not good enough
//    char* c = DISK_INODE;
//    for (; c<DISK_END; c++) {
//        int k=0;
//        for (k=0; k<8; k++) {
//            if (((*c) & (1<<k)) == 0) {
//                uint32_t ans = transform(c, k);
//                *c = *c | (1<<k);
//                return ans;
//            }
//        }
//    }
    panic("there is no room left for creating a new file...\n");
    return 0xffffffff;
}

static void alloc_for_file(File* file) {
    int cur = 0;
    for (cur=0; cur<10; cur+=1) {
        file->inodes[cur] = find_empty_inode();
    }
}

static void alloc_for_indirect(File* file) {
    file->idirect = find_empty_inode();
    uint32_t* indirectBlock = getinodeaddress(file->idirect);
    int counter = 0;
    while (counter < (1<<10)) {
        *(indirectBlock + counter) = find_empty_inode();
    }
}

// ================ outer functions =========================


char* pwd() { // show where are we?
    int i=0;
    for (;i<depth; i++) {
        strcat(pathbuffer, where[i]->filename);
        strcat(pathbuffer, "/");
    }
    return pathbuffer;
}

int cd(const char* foldername) {
    if (strcmp(foldername, "..") == 0) {
        if (depth == 1) {
            return -1;
        } else {
            --depth;
            return 1;
        }
    }

    File* next = getfilefromfolderbyname(where[depth-1], foldername);

    if (!next || next->is_dir == 0) {
        return -1; // failed to execute
    } else {
        where[depth++] = next;
        return 1;
    }
}

int mkdir(const char* foldername) {
    File* target = getfilefromfolderbyname(where[depth-1], foldername);
    if (target) {
        return -1;
    } else {
        return creatfile(where[depth-1], foldername, READ | WRITE, 1);
    }
}

int creat(const char* filename) {
    File* target = getfilefromfolderbyname(where[depth-1], filename);
    if (target) {
        return -1;
    } else {
        return creatfile(where[depth-1], filename, READ | WRITE, 0);
    }
}



void cat(const char* filename) {
    File* target = getfilefromfolderbyname(where[depth-1], filename);
    if (!target || target->is_dir == 1) {
        printf("this file doesn't exist or a invalid file.\n");
    } else {
        if (target->filesize < PGSIZE * 10) {

            int no = target->filesize / PGSIZE;
            int left = target->filesize - no * PGSIZE;
            int k;
            for (k=0; k<no; k++) display(getinodeaddress(target->inodes[k]), PGSIZE);
            display(getinodeaddress(target->inodes[no]), left);

        } else {
            printf("This file is too large to display");
        }
    }
}

void ls() {

    File* now = where[depth-1];
    int i;

    for (i=0;i<now->filesize; i++) {
        File* temp = getfilefromfolderbyno(now, i);
        printf("%10s%5d%5d\n", temp->filename, temp->filesize, temp->is_dir);
    }
}

void init_map() {

    char* begin = (char*)DISK_MAP;
    char* end = (char*)DISK_INODE;
    search_begin = begin;
    for (;begin!=end; begin++) *begin = 0;
    // the File struct of root is in the super block

    File* rootinfo = (File*)(DISK_SUPER);
    strcpy(rootinfo->filename, "root");
    rootinfo->filesize = 0;
    alloc_for_file(rootinfo);
    rootinfo->idirect = 0;
    rootinfo->is_dir = 1;
    rootinfo->permission = READ | WRITE;

    where[0] = rootinfo;
    depth = 1;
}

int creatfile(File* dir, const char* name,  uint32_t perm, int is_folder) {
    assert(dir->is_dir==1, "argument must be directory\n");
    int sz = dir->filesize;
    if (sz == 160) alloc_for_indirect(dir);
    // which indacate that this is full...
    if (sz == 160 + 16000) return -1;
    File* begin = getfilefromfolderbyno(dir, sz);
    strcpy(begin->filename, name);
    begin->filesize = 0;
    begin->is_dir = is_folder;
    begin->permission = perm;
    begin->idirect = 0;

    dir->filesize += 1;
    alloc_for_file(begin);
    return 1;
}


int writefile(const char* filename, const char* buffer, uint32_t len) {
    File* target = getfilefromfolderbyname(where[depth-1], filename);
    if (!target || target->is_dir) return -1;
    if (len > 10 * PGSIZE + PTSIZE) return -1; // too large
    target->filesize = len;
    if (len < 10 * PGSIZE) {
        int no = target->filesize / PGSIZE;
        int left = target->filesize - no * PGSIZE;
        int k;
        for (k=0; k<no; k++) memcpy(getinodeaddress(target->inodes[k]),buffer + PGSIZE * k , PGSIZE);
        memcpy(getinodeaddress(target->inodes[no]), buffer + PGSIZE * no, left);

    } else {
        int k;
        alloc_for_indirect(target);
        for (k=0; k<10; k++) memcpy(getinodeaddress(target->inodes[k]),buffer + PGSIZE * k , PGSIZE);
        buffer = buffer + 10 * PGSIZE;
        uint32_t* indir = getinodeaddress(target->idirect);
        int need = len - 10 * PGSIZE;
        int no = need / PGSIZE;
        int left = need % PGSIZE;
        for (k=0; k<no; k++) memcpy(indir[k], buffer + PGSIZE * k, PGSIZE);
        memcpy(indir[no], buffer + PGSIZE * no, left);
        // TODO: need test.
        return 1;
    }

    return 1;
}


int main() {
    DISK_SUPER = malloc(1<<30);
}

