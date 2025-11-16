#include "kernel/types.h"
#include "user/user.h"

#define N 280

void primes(int) __attribute__((noreturn));

void primes(int read_fd) {
    int first, x;
    int newpipe[2];

    if (read(read_fd, &first, sizeof(int)) <= 0) { 
        close(read_fd);
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

    if (pipe(p) < 0) {
        printf("Initial pipe failed\n");
        exit(1);
    }

    int pid = fork();
    if (pid < 0) {
        printf("Initial fork failed\n");
        close(p[0]);
        close(p[1]);
        exit(1);
    }

    if (pid == 0) {
        close(p[1]);
        primes(p[0]);
    } else {
        close(p[0]);
        for (int i = 2; i <= N; i++) {
            write(p[1], &i, sizeof(int));
        }

        close(p[1]);
        wait(0);
    }
    exit(0);
}
