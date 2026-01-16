#include "kernel/types.h"
#include "user/user.h"

#define N 280

void primes(int) __attribute__((noreturn));

void primes(int read_fd) {
    int first, x; // first: số đầu tiên đọc được → số nguyên tố, x: các số tiếp theo
    int newpipe[2]; // pipe mới để truyền các số không chia hết cho first

    if (read(read_fd, &first, sizeof(int)) <= 0) { // Đọc 1 số nguyên từ pipe read_fd và lưu vào biến first
        close(read_fd); // 0 EOF – pipe đã bị đóng, hết dữ liệu, < 0 Lỗi
        exit(0);
    }

    printf("prime %d\n", first);

    if (pipe(newpipe) < 0) {
        printf("Pipe creation failed\n");
        close(read_fd);
        exit(1);
    }

    int pid = fork();
    if (pid < 0) {
        printf("Fork failed\n");
        close(read_fd);
        close(newpipe[0]);
        close(newpipe[1]);
        exit(1);
    }

    if (pid == 0) {
        close(newpipe[1]);
        close(read_fd);
        primes(newpipe[0]);
    }

    close(newpipe[0]);

    while (read(read_fd, &x, sizeof(int)) > 0) {
        if (x % first != 0) {
            write(newpipe[1], &x, sizeof(int));
        }
    }

    close(newpipe[1]);
    close(read_fd);

    wait(0);
    exit(0);
}

int main() {
    int p[2];
    // p[0]: đầu đọc pipe
    // p[1]: đầu ghi pipe

    if (pipe(p) < 0) {
        printf("Initial pipe failed\n");
        exit(1);// thoát do lỗi
    }

    int pid = fork(); // tao tiến trình con

    if (pid < 0) { // lỗi khi tao tien trinh con
        printf("Initial fork failed\n");
        close(p[0]);
        close(p[1]);
        exit(1);
    }

    if (pid == 0) { // tiến trình con
        close(p[1]); // đóng đầu ghi pipe, tránh treo read, con không ghi chỉ đọc
        primes(p[0]);
    } else { // tiến trình cha
        close(p[0]); // đóng đầu đọc pipe
        for (int i = 2; i <= N; i++) {
            write(p[1], &i, sizeof(int));
        }

        close(p[1]);// đóng đầu ghi pipe sau khi gửi hết số , Báo EOF cho tiến trình con
        wait(0); // Cha đợi tiến trình con kết thúc, Tránh zombie process
    }
    exit(0);// thoát thành công
}

/* Zombie process là: Một tiến trình đã kết thúc (exit) nhưng vẫn còn tồn tại 
trong bảng tiến trình vì cha chưa thu dọn trạng thái của nó. Chiếm 1 entry trong process table */