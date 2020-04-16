#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <assert.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <time.h>
extern char** environ;
struct syscallNameAndTime{
  char name[50];
  double time;
};
int cmp(const void* s1, const void* s2) {
  return (*(struct syscallNameAndTime *)s1).time>(*(struct syscallNameAndTime *)s2).time ? -1 : 1;
  
}
int myReadLine(int fd, char* line) {
  char ch;
  int offset = 0;
  while(read(fd,&ch,1)>0) {
    line[offset] = ch;
    if (ch == '\n') {
      line[offset] = '\0';
      return 1;
    }
    if (ch == EOF) {
      line[offset] = '\0';
      return 0;
    }
    offset += 1;
  }
  return -1;
}
int main(int argc, char *argv[]) {

  struct syscallNameAndTime syscallList[1000];
  for (int i = 0; i < 1000; i++) {
    strcpy(syscallList[i].name, "NONE");
    syscallList[i].time = 0;
  }


  // prepare for trace system call
  char** cmdArgs = malloc(sizeof(char*)*(argc + 2));
  cmdArgs[0] = "strace";
  cmdArgs[1] = "-T";
  for (int i = 1; i <= argc; i++){
    cmdArgs[i+1] = argv[i];
  }
  char *pathvar = getenv("PATH"); 
  char PATH[512] = {0};
  strcpy(PATH, getenv("PATH"));
  char* path = malloc(sizeof(char)*(strlen("PATH=") + strlen(pathvar)+1));
  memset(path, '\0', strlen("PATH=") + strlen(pathvar)+1);
  strcat(path, "PATH=");
  strcat(path, pathvar);
  char *exec_envp[] = { path, NULL, };
  char *test[] = { "strace", "-T", "ls", NULL, };
  // execute program

  char** env = environ;
  int pipefds[2];
	if(pipe(pipefds) < 0){
		perror("pipe");
    assert(0);
	}
  pipe(pipefds);
  int pid = -1;
  pid = fork();
  if (pid == 0) {
    close(pipefds[0]);
    dup2(pipefds[1], fileno(stderr));
    int fd = open("/dev/null",O_RDWR);
    dup2(fd, fileno(stdout));
    // 子进程，执行strace命令
    char*token = strtok(PATH, ":");
    char stracePath[100];
    memset(stracePath, '\0', 100);
    strcat(stracePath, token);
    strcat(stracePath, "/strace");
    printf("%s\n", stracePath);
    while((execve(stracePath, cmdArgs, env)) == -1){
      memset(stracePath, '\0', 100);
      strcat(stracePath, strtok(NULL, ":"));
      strcat(stracePath, "/strace");
      printf("%s\n", stracePath);
    }
    assert(0);
    // 不应该执行此处代码，否则execve失败，出错处理
  } else {
    int listLen = 0;
    double totalTime = 0;
    close(pipefds[1]);
    char buf[512];
    int pre = clock();
    while(myReadLine(pipefds[0], buf) > 0) {
      int now = clock();
      if (now - pre >= CLOCKS_PER_SEC) {
        qsort(syscallList, listLen, sizeof(struct syscallNameAndTime), cmp);
        for (int i = 0; i<5 && i < listLen; i++) {
          printf("%s (%d%%)\n", syscallList[i].name, (int) ((syscallList[i].time/totalTime)*100));
        }
        printf("==================\n");
        for (int i = 0; i < 80; i++) {
          printf("%c",'\0');
        }
        fflush(stdout);
        pre = now;
      }
      int len = strlen(buf);
      int left = -1;
      int right = len-1;
      int leftparameter = -1;
      for (int i = len-1; i >= 0; i--) {
        if (buf[i] == '<') {
          left = i;
          break;
        }
      }
      for (int i = 0; i < len; i++) {
        if (buf[i] == '(') {
          leftparameter = i;
          break;
        }
      }
      if (buf[right]=='>'){
        char time[100];
        char syscall[50];
        memset(syscall, '\0', 50);
        memset(time, '\0', 100);
        if (leftparameter > 0&&('a'<=buf[0] && 'z'>=buf[0])){
          memcpy(time, &buf[left+1], (right-left-1));
          memcpy(syscall, &buf[0], leftparameter);
          double dtime = strtod(time, NULL);
          for (int i = 0; i < len; i++) {
            if (strcmp(syscallList[i].name, "NONE") != 0) {
              if (strcmp(syscallList[i].name, syscall) == 0) {
                syscallList[i].time += dtime;
                totalTime += dtime;
                break;
              }
            }
            if (strcmp(syscallList[i].name, "NONE") == 0) {
              listLen += 1;
              strcpy(syscallList[i].name, syscall);
              syscallList[i].time = dtime;
              totalTime += dtime;
              assert(syscallList[i].name != NULL);
              break;
            }
          }
        }
      }
      memset(buf, '\0', sizeof(buf));
    }
    qsort(syscallList, listLen, sizeof(struct syscallNameAndTime), cmp);
    for (int i = 0; i<5 && i < listLen; i++) {
      printf("%s (%d%%)\n", syscallList[i].name, (int) ((syscallList[i].time/totalTime)*100));
    }
    printf("==================\n");
    for (int i = 0; i < 80; i++) {
      printf("%c",'\0');
    }
    fflush(stdout);
    return 0;
  }
  return 0;
  perror(argv[0]);
  exit(EXIT_FAILURE);
}
