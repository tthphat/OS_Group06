#include "kernel/types.h"
#include "user/user.h"

#define N 280

void primes(int) __attribute__((noreturn));

void primes(int read_fd) {
    int first, x;
    int newpipe[2];

    // đọc số đầu tiên trong pipe
    if (read(read_fd, &first, sizeof(int)) <= 0) { // hết dữ liệu hoặc lỗi
        close(read_fd);
        exit(0);
    }

    printf("prime %d\n", first);

    // pipe mới cho stage kế tiếp
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
        // child: chỉ đọc từ newpipe
        close(newpipe[1]);
        close(read_fd);
        primes(newpipe[0]);
    }

    // parent: lọc rồi ghi vào newpipe[1]
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

    if (pipe(p) < 0) {
        printf("Initial pipe failed\n");
        exit(1);
    }
    //Khi bạn gọi pipe(p), kernel sẽ: Tạo một vùng nhớ đệm (buffer) dùng để lưu dữ liệu tạm thời giữa 2 đầu của pipe
    // sau đó, Tạo hai đối tượng "file" nội bộ:
    // Một cho đầu đọc (read end)
    // Một cho đầu ghi (write end)

    // Mỗi đối tượng đó chứa:
    // Con trỏ tới vùng nhớ đệm pipe
    // Cờ trạng thái (đọc/ghi)
    // Bộ đếm vị trí đọc/ghi
    // Thông tin đồng bộ (ai đang chờ đọc, ai đang chờ ghi, v.v.)

    //Sau đó kernel thêm 2 entry vào bảng file descriptor của tiến trình hiện tại và gán:
    // p[0] là file descriptor để đọc từ pipe
    // p[1] là file descriptor để ghi vào pipe
    // p[0] không chứa dữ liệu, mà chứa một số nguyên trỏ tới một entry trong kernel.

    int pid = fork();
    if (pid < 0) {
        printf("Initial fork failed\n");
        close(p[0]);
        close(p[1]);
        exit(1);
    }

    if (pid == 0) {
        // child: primes stage
        close(p[1]);
        primes(p[0]);
    } else {
        // parent: generate numbers
        close(p[0]);
        for (int i = 2; i <= N; i++) {
            write(p[1], &i, sizeof(int));
        }

        close(p[1]);
        wait(0);
    }
    // a) Sau fork()
    // Cha: đóng p[0], bắt đầu ghi các số 2..N.
    // Con: đóng p[1], gọi primes(p[0]), bắt đầu đọc.
    // b) Nếu con chạy trước
    // Con gọi read(p[0], &x, sizeof(int))
    // Kernel thấy pipe trống → block (đợi) cho đến khi có dữ liệu.
    // Khi cha bắt đầu write, kernel tự đánh thức con dậy → read() thành công.

    exit(0);
}
