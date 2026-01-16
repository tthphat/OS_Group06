#include "kernel/types.h"
#include "kernel/stat.h"
#include "user/user.h"
#include "kernel/fs.h"
#include "kernel/param.h"

int
main(int argc, char *argv[]){
    int index = 0; // vị trí tên lệnh trong argv
    int _argc = 1; // số đối số hiện tại trong _argv
    char *_argv[MAXARG]; // mảng đối số mới truyền cho exec()

    if(strcmp(argv[1],"-n")==0){
        index = 3;
    }else{
        index = 1;
    }

    _argv[0] = malloc(strlen(argv[index])+1); // Cấp phát bộ nhớ cho đối số đầu tiên
    strcpy(_argv[0],argv[index]); // Sao chép tên lệnh vào _argv[0]

    for(int i=index+1;i<argc;++i){ // Sao chép các đối số từ argv sang _argv
        //printf("--%s--\n",argv[i]); 
        _argv[_argc] = malloc(strlen(argv[i])+1);
        strcpy(_argv[_argc++],argv[i]);
    }
    _argv[_argc] = malloc(128); // Cấp phát bộ nhớ cho đối số cuối cùng (chuỗi nhập từ stdin)
 
    char buf; // Biến tạm để đọc 1 ký tự từ stdin
    int i =0;
    while(read(0,&buf,1)){
        if(buf=='\n'){
            _argv[_argc][i++]='\0'; // Kết thúc chuỗi nhập
            if(fork()==0){ // Tạo tiến trình con để thực thi lệnh với đối số mới
                exec(argv[index],_argv); // Thực thi lệnh với đối số trong _argv
            }else{
                i=0; // reset buffer cho chuỗi nhập mới
                wait(0);
            }
        }else{
            _argv[_argc][i++]=buf; // Thêm ký tự vào chuỗi nhập
        }
    }
    exit(0);
}
