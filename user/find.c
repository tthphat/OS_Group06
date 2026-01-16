#include "kernel/types.h"
#include "kernel/stat.h"
#include "user/user.h"
#include "kernel/fs.h"
#include "kernel/fcntl.h"

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

    if(fstat(fd, &st) < 0){ // lấy thông tin của file hoặc thư mục vừa mở nhằm xác định nó là file hay thư mục, và dừng xử lý nếu không lấy được thông tin.
        fprintf(2, "ls: cannot stat %s\n", path);
        close(fd);
        return;
    }

    // st.type	Loại file (T_FILE, T_DIR, T_DEVICE)
    // st.ino	inode number
    // st.size	kích thước file

    switch(st.type){
    case T_DEVICE: // không dừng nếu không có break, nên cũng sẽ print
    case T_FILE:
            printf("%s\n", path);
        break;

    case T_DIR:
        if(strlen(path) + 1 + DIRSIZ + 1 > sizeof buf){ // buf chỉ có 512 byte
            printf("ls: path too long\n");
            break;
        }

        strcpy(buf, path);
        p = buf+strlen(buf);
        *p++ = '/'; // adding "/" to the last cstring path. [E.g: $ ./ ]

        while(read(fd, &de, sizeof(de)) == sizeof(de)){ // read file or dir in current directory
            if(de.inum == 0) // chỗ trống trong thư mục
                continue;
            memmove(p, de.name, DIRSIZ); // Copy đúng DIRSIZ byte của tên file con vào sau path/
            p[DIRSIZ] = 0; // Thêm ký tự kết thúc chuỗi '\0' vào cuối tên file
            if(stat(buf, &st) < 0){
                printf("ls: cannot stat %s\n", buf);
                continue;
            }

            if(st.type == T_DIR && strcmp(de.name, ".") != 0  // Không phải thư mục hiện tại
                && strcmp(de.name, "..") != 0) { // Không phải thư mục cha
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