// user/pingpong.c
#include "kernel/types.h"
#include "kernel/stat.h"
#include "user/user.h"

int main(void)
{
    int p_parent_to_child[2];
    int p_child_to_parent[2];
    char buf;

    // Tao hai pipe: mot cho parent->child, mot cho child->parent
    if (pipe(p_parent_to_child) < 0 || pipe(p_child_to_parent) < 0) {
        fprintf(2, "pipe failed\n");
        exit(1);
    }

    int pid = fork();
    if (pid < 0) {
        fprintf(2, "fork failed\n");
        exit(1);
    }

    if (pid == 0) {
        // --- Child ---
        close(p_parent_to_child[1]); // dong dau ghi
        close(p_child_to_parent[0]); // dong dau doc

        // Doc 1 byte tu parent
        if (read(p_parent_to_child[0], &buf, 1) != 1) {
            exit(1);
        }
        printf("%d: received ping\n", getpid());

        // Gui lai 1 byte cho parent
        write(p_child_to_parent[1], "x", 1);

        close(p_parent_to_child[0]);
        close(p_child_to_parent[1]);
        exit(0);
    } else {
        // --- Parent ---
        close(p_parent_to_child[0]); // dong dau doc
        close(p_child_to_parent[1]); // dong dau ghi

        // Gui 1 byte toi child
        write(p_parent_to_child[1], "x", 1);

        // Doc phan hoi tu child
        read(p_child_to_parent[0], &buf, 1);
        printf("%d: received pong\n", getpid());

        close(p_parent_to_child[1]);
        close(p_child_to_parent[0]);
        wait(0);
        exit(0);
    }
}
