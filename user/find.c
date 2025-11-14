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
  memmove(buf, p, strlen(p)); // buff = p (deep copy)
  memset(buf+strlen(p), ' ', DIRSIZ-strlen(p)); 
  return buf;
}

void
find(char* path, char* name) {
    char buf[512], *p;
    int fd;
    struct dirent de;
    struct stat st;

    if((fd = open(path, O_RDONLY)) < 0){
        fprintf(2, "ls: cannot open %s\n", path);
        return;
    }

    if(fstat(fd, &st) < 0){
        fprintf(2, "ls: cannot stat %s\n", path);
        close(fd);
        return;
    }

    switch(st.type){
    case T_DEVICE:
    case T_FILE:
            printf("%s\n", path);
        break;

    case T_DIR:
        if(strlen(path) + 1 + DIRSIZ + 1 > sizeof buf){
            printf("ls: path too long\n");
            break;
        }

        strcpy(buf, path);
        p = buf+strlen(buf);
        *p++ = '/'; // adding "/" to the last cstring path. [E.g: $ ./ ]

        while(read(fd, &de, sizeof(de)) == sizeof(de)){ // read file or dir in current directory
            if(de.inum == 0)
                continue;
            memmove(p, de.name, DIRSIZ);
            p[DIRSIZ] = 0;
            if(stat(buf, &st) < 0){
                printf("ls: cannot stat %s\n", buf);
                continue;
            }

            if(st.type == T_DIR && strcmp(de.name, ".") != 0 
                && strcmp(de.name, "..") != 0) {
                    find(buf, name);
            }

            if(st.type == T_FILE && strcmp(de.name, name) == 0) {
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
    if(argc < 3 || 4 < argc) {
        printf("Invalid find(<path>,<filename>)\n");
        exit(0);
    }
    find(argv[1], argv[2]);
    exit(0);
}