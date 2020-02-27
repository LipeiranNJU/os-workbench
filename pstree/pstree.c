#include <stdio.h>
#include <assert.h>
#include <string.h>
#include <stdbool.h>
#include <unistd.h>
#include <dirent.h>
#include <stdlib.h>
#include <assert.h>

struct data{
    char name[100];
    int pid;
    int ppid;
    int layer;
};

int compute_layer(struct data* a, struct data* list) {
    if (a->layer >= 0)
        return a->layer;
    if (a->ppid == 0) 
        return 0;
    else
    {
        struct data* pdata;
        for (pdata = list; pdata->pid != a->ppid; pdata++);
        assert(pdata->pid == a->ppid);
        a->layer = compute_layer(pdata, list) + 1;
        return a->layer;
    }
    
}

void showdir(char* path_of_dir) {
	DIR* dir;
	struct dirent *ptr;
	dir = opendir(path_of_dir); 
	while((ptr = readdir(dir)) != NULL) {
		printf("%s is in the directory %s.\n", ptr->d_name, path_of_dir);
	}
}

int my_filter(const struct dirent* dir) {
	if ((dir->d_name)[0] >= '0' && (dir->d_name)[0]<='9')
		return 1;
	else
		return 0;
}



int main(int argc, char *argv[]) {
    typedef bool pstree_option;
    pstree_option pstree_show_pids, pstree_numeric_sort, pstree_version;
    pstree_show_pids = pstree_numeric_sort = pstree_version = false;
    for (int i  = 0; i < argc; i++) {
        assert(argv[i]);
//        printf("argv[%d] = %s\n", i, argv[i]);
        if (strcmp(argv[i], "-p") == 0 || strcmp(argv[i], "--show-pids") == 0 || argc == 1)
            pstree_show_pids = true;
        if (strcmp(argv[i], "-n") == 0 || strcmp(argv[i], "numeric-sort") == 0)
            pstree_numeric_sort = true;
        if (strcmp(argv[i], "-V") == 0 || strcmp(argv[i], "--version") == 0)
            pstree_version = true; 
    }
   // pid_t pid = getpid();
   // printf("Pid:%d\n",pid);

      if (pstree_version == true) {
          printf("pstree (PSmisc) 23.1\nCopyright (C) 1993-2017 Werner Almesberger and Craig Small\n\nPSmisc 不提供任何保证。\n该程序为自由软件，欢迎你在 GNU 通用公共许可证 (GPL) 下重新发布。\n详情可参阅 COPYING 文件。\n");
          return 0;
      }


	struct dirent **namelist;
	int n;
	n = scandir("/proc", &namelist, my_filter, alphasort);
    int nn = n;
    char current_path[100];
    char one_line[100];
    struct data list[nn];
    printf("NN:%d\n", nn);
    // assert(0);
    for (int i = 0; i < nn; i++) {
        strcat(strcat(strcpy(current_path,"/proc/"), namelist[i]->d_name), "/status");
        FILE* fp = fopen(current_path, "r");
        fgets(one_line, 100, fp);
        strcpy(list[i].name, one_line + 6);
        fgets(one_line, 100, fp);
        fgets(one_line, 100, fp);
        fgets(one_line, 100, fp);
        fgets(one_line, 100, fp);
        fgets(one_line, 100, fp);//pid
        list[i].pid = atoi(one_line + 5);
        fgets(one_line, 100, fp);//ppid
        list[i].ppid = atoi(one_line + 6);
        list[i].layer = -1;

    }
    for (int i = 0; i < nn; i++) {
        list[i].layer = compute_layer(&list[i], list);
    }
    
    
    if (pstree_show_pids == true) {
        struct data tmp;
        for (int i = 0; i < nn - 1; i++) {
            for (int j = 0; j < nn - 1 - i; j++) {
                if (list[j].layer > list[j + 1].layer) {
                    strcpy(tmp.name, list[j].name);
                    tmp.pid = list[j].pid;
                    tmp.ppid = list[j].ppid;
                    tmp.layer = list[j].layer;

                    strcpy(list[j].name, list[j+1].name);
                    list[j].pid = list[j+1].pid;
                    list[j].ppid = list[j+1].ppid;
                    list[j].layer = list[j+1].layer;

                    strcpy(list[j+1].name, tmp.name);
                    list[j+1].pid = tmp.pid;
                    list[j+1].ppid = tmp.ppid;
                    list[j+1].layer = tmp.layer;
                }
            }
        }
        for (int i = 0; i < nn; i++) {
            printf("layer:%d\t(pid%d)(ppid%d)%s",list[i].layer, list[i].pid, list[i].ppid, list[i].name);
        }
        // printf("show-pids\n");
        // assert(0);
        return 0;
    }
    if (pstree_numeric_sort == true) {

        struct data tmp;
        for (int i = 0; i < nn - 1; i++) {
            for (int j = 0; j < nn - 1 - i; j++) {
                if (strcmp(list[j].name, list[j + 1].name)) {
                    strcpy(tmp.name, list[i].name);
                    tmp.pid = list[j].pid;
                    tmp.ppid = list[j].ppid;

                    strcpy(list[j].name, list[j+1].name);
                    list[j].pid = list[j+1].pid;
                    list[j].ppid = list[j+1].ppid;

                    strcpy(list[j+1].name, tmp.name);
                    list[j+1].pid = tmp.pid;
                    list[j+1].ppid = tmp.ppid;
                }
            }
        }
        for (int i = 0; i < nn; i++) {
            printf("(%d)%s",list[i].pid, list[i].name);
        }


        for (int i = 0; i < nn; i++) {
            printf("%s",list[i].name);
        }
        // printf("numeric-sort\n");
    }
    assert(!argv[argc]);
    return 0;
}

