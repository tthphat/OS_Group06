#include "kernel/types.h"   // Kiểu dữ liệu cơ bản
#include "kernel/stat.h"    // struct stat
#include "user/user.h"      // syscall: read, fork, exec, wait, malloc
#include "kernel/fs.h"      // filesystem (không dùng trực tiếp nhưng thường include)
#include "kernel/param.h"   // MAXARG: số lượng argument tối đa cho exec

int
main(int argc, char *argv[])
{
    int index = 0;          // Vị trí của command cần exec trong argv
    int _argc = 1;          // Số lượng argument cho exec (bắt đầu từ 1 vì argv[0] là tên lệnh)
    char *_argv[MAXARG];    // Mảng argument mới truyền cho exec

    // Kiểm tra có option "-n" hay không
    // Nếu có "-n", cấu trúc lệnh là: xargs -n <num> <command>
    if(strcmp(argv[1], "-n") == 0){
        index = 3;          // command nằm ở argv[3]
    } else {
        index = 1;          // command nằm ở argv[1]
    }

    // _argv[0] luôn là tên chương trình cần exec
    _argv[0] = malloc(strlen(argv[index]) + 1);
    strcpy(_argv[0], argv[index]);

    // Copy các tham số có sẵn sau command (nếu có)
    for(int i = index + 1; i < argc; ++i){
        _argv[_argc] = malloc(strlen(argv[i]) + 1);
        strcpy(_argv[_argc++], argv[i]);
    }

    // Chuẩn bị vùng nhớ cho argument đọc từ stdin
    // Mỗi dòng stdin sẽ trở thành 1 argument mới
    _argv[_argc] = malloc(128);

    char buf;   // Biến đọc từng ký tự từ stdin
    int i = 0;  // Chỉ số ghi vào argument cuối

    // Đọc stdin từng byte một
    while(read(0, &buf, 1)){
        // Nếu gặp ký tự xuống dòng
        if(buf == '\n'){
            // Kết thúc chuỗi argument
            _argv[_argc][i++] = '\0';

            // Tạo process con
            if(fork() == 0){
                // Process con: chạy command với argv đã xây dựng
                exec(argv[index], _argv);
            } else {
                // Process cha: reset chỉ số
                i = 0;

                // Chờ process con chạy xong
                wait(0);
            }
        } else {
            // Nếu chưa gặp newline, tiếp tục ghép ký tự vào argument
            _argv[_argc][i++] = buf;
        }
    }

    // Kết thúc chương trình
    exit(0);
}
