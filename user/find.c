#include "kernel/types.h"
#include "kernel/stat.h"
#include "user/user.h"
#include "kernel/fs.h"
#include "kernel/fcntl.h"

char*
fmtname(char *path)
{
    static char buf[DIRSIZ+1];
    char *p;

    // Find first character after last slash.
    for(p=path+strlen(path); p >= path && *p != '/'; p--)
        ;
    p++;

    // Return blank-padded name.
    if(strlen(p) >= DIRSIZ)
        return p;
    memmove(buf, p, strlen(p));
    memset(buf+strlen(p), ' ', DIRSIZ-strlen(p));
    return buf;
}

void
find(char *path, char *str) {

    char buf[512], *p;
    struct dirent de;
    struct stat st;
    int fd;

    fd = open(path, O_RDONLY);
    fstat(fd, &st);

    switch(st.type){
        case T_DIR:
            strcpy(buf, path);
            p = buf+strlen(buf);
            *p++ = '/';

            while(read(fd, &de, sizeof(de)) == sizeof(de)){
                if(de.inum == 0)
                    continue;
                memmove(p, de.name, DIRSIZ);
                p[DIRSIZ] = 0;

                stat(buf, &st);

                if (st.type == T_DIR && *p != '.') {
                    find(buf, str);
                } else if (strcmp(p, str) == 0) {
                    printf("%s\n", buf);
                }
            }
            break;
    }
    close(fd);
}

int
main(int argc, char *argv[])
{
    if(argc < 3){
        fprintf(2, "Usage: find <path> <file to find>\n");
        exit(-1);
    }

    find(argv[1], argv[2]);
    exit(0);
}


