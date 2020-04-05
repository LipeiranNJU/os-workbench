#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <assert.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
int main(int argc, char *argv[]) {
  // prepare for trace system call
  char** cmdArgs = malloc(sizeof(char*)*(argc + 2));
  cmdArgs[0] = "strace";
  cmdArgs[1] = "-T";
  for (int i = 1; i <= argc; i++){
    cmdArgs[i+1] = argv[i];
  }
  char *pathvar = getenv("PATH"); 
  char* path = malloc(sizeof(char)*(strlen("PATH=") + strlen(pathvar)+1));
  memset(path, '\0', strlen("PATH=") + strlen(pathvar)+1);
  strcat(path, "PATH=");
  strcat(path, pathvar);
  char *exec_envp[] = { path, NULL, };
  char *test[] = { "strace", "-T", "ls", NULL, };
  // execute program

  int pipefds[2];
	if(pipe(pipefds) < 0){
		perror("pipe");
    assert(0);
	}
  pipe(pipefds);
  int pid = -1;
  pid = fork();
  if (pid == 0) {
    dup2(pipefds[1], fileno(stderr));
    int fd = open("/dev/null",O_RDWR);
    dup2(fd, fileno(stdout));
    // 子进程，执行strace命令
    execve("/usr/bin/strace", cmdArgs, exec_envp);
    printf("AA\n");
    assert(0);
    
    // 不应该执行此处代码，否则execve失败，出错处理
  } else {
    close(pipefds[1]);
    char buf[4];
    while(read(pipefds[0], buf, sizeof(buf)-1) > 0) {
      write(fileno(stdout), buf, strlen(buf));
      printf("DDD\n");
      memset(buf, '\0', sizeof(buf));
    }
    printf("$$$\n");

    // 父进程，读取strace输出并统计
    // printf("BBB\n");
    // assert(0);
    return 0;
  }
  return 0;
  perror(argv[0]);
  exit(EXIT_FAILURE);
}
