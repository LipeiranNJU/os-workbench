#include <stdio.h>
#include <assert.h>
#include <string.h>
#include <stdbool.h>
#include <unistd.h>
#include <dirent.h>
#include <stdlib.h>
#include <assert.h>

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
	while(n--) {
		printf("test scandir:%s\n", namelist[n]->d_name);
		// free(namelist[n]);
        // assert(n != 0);
	} 
    char current_path[100];
    char one_line[100];
    for (int i = 0; i < nn; i++) {
        strcat(strcat(strcpy(current_path,"/proc/"), namelist[i]->d_name), "/status");
        // printf("current status path is %s\n", current_path);
        // memset(current_path, '\0', 100);
        FILE* fp = fopen(current_path, "r");
        fgets(one_line, 100, fp);
        printf("Head line:\n%s", one_line + 6);
        fgets(one_line, 100, fp);
        fgets(one_line, 100, fp);
        fgets(one_line, 100, fp);
        fgets(one_line, 100, fp);
        fgets(one_line, 100, fp);//pid
        printf("%s", one_line + 5);
        fgets(one_line, 100, fp);//ppid
        // printf("%s\n", one_line + 6);
        strtok(one_line, "\t");
        strtok(NULL, "\t");
        printf("ppid:%s", strtok(NULL, "\t"));

    }


    DIR *dir;
    char initial_path[] = "/proc";
    struct dirent *ptr;
    struct dirent *ptr2;

    dir = opendir("/proc");
    while((ptr = readdir(dir)) != NULL) {
	    if (ptr->d_name [0]>='0' && ptr->d_name[0] <= '9') {
		// 	printf("dir name:%s\n", ptr->d_name);
			char process_dir[100];
			strcat(strcat(strcpy(process_dir, initial_path),"/"),(ptr->d_name));
		//	printf("dir path:%s\n",process_dir);
		//	DIR *dir2;
		//	dir2 = opendir(process_dir);
		//	while ((ptr2 = readdir(dir2)) != NULL) {
		//		printf("process name:%s\n", ptr2->d_name);

		//	}
		//	showdir(process_dir);
		//	break;
	 	}
    } 
    closedir(dir);
    
    if (pstree_show_pids == true) {
        printf("show-pids\n");
    }
    if (pstree_numeric_sort == true) {
        printf("numeric-sort\n");
    }
    assert(!argv[argc]);
    return 0;
}

