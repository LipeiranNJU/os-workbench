#include <stdio.h>
#include <assert.h>
#include <string.h>
#include <stdbool.h>
#include <unistd.h>
#include <dirent.h>

int main(int argc, char *argv[]) {
    typedef bool pstree_option;
    pstree_option pstree_show_pids, pstree_numeric_sort, pstree_version;
    pstree_show_pids = pstree_numeric_sort = pstree_version = false;
    for (int i = 0; i < argc; i++) {
        assert(argv[i]);
        printf("argv[%d] = %s\n", i, argv[i]);
        if (strcmp(argv[i], "-p") == 0 || strcmp(argv[i], "--show-pids") == 0 || argc == 1)
            pstree_show_pids = true;
        if (strcmp(argv[i], "-n") == 0 || strcmp(argv[i], "numeric-sort") == 0)
            pstree_numeric_sort = true;
        if (strcmp(argv[i], "-V") == 0 || strcmp(argv[i], "--version") == 0)
            pstree_version = true;
    }
    pid_t pid = getpid();
    printf("Pid:%d\n",pid);

    DIR *d;
    

    if (pstree_version == true) {
        printf("pstree (PSmisc) 23.1\nCopyright (C) 1993-2017 Werner Almesberger and Craig Small\n\nPSmisc 不提供任何保证。\n该程序为自由软件，欢迎你在 GNU 通用公共许可证 (GPL) 下重新发布。\n详情可参阅 COPYING 文件。\n");
        return 0;
    }
    if (pstree_show_pids == true) {
        printf("show-pids\n");
    }
    if (pstree_numeric_sort == true) {
        printf("numeric-sort\n");
    }
    assert(!argv[argc]);
    return 0;
}
// It is a test for github
// Ignore it

// new test