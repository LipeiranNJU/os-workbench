#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
int main(int argc, char *argv[]) {
  char* cmd = argv[1];
  char ** cmdArgsRaw = NULL;
  char* cmdArgs = NULL;
  if (argc >= 2) {
    cmdArgsRaw = &argv[2];
    printf("%s\n", cmd);
    for (int i = 0; i < argc-2; i++) {
      printf("%s\n", cmdArgsRaw[i]);
    }
  }
  if (cmdArgsRaw != NULL) {
    int len = 0;
    printf("AAA\n");
    for (int i = 0; i < argc-2; i++) {
      len += strlen(cmdArgsRaw[i]);
      len += 1;
    }
    printf("len:%d\n",len);
    cmdArgs = malloc(len);
    memset(cmdArgs, '\0', len);
    printf("BBB\n");
    for (int i = 0; i < argc-3; i++) {
      strcat(cmdArgs, cmdArgsRaw[i]);
      strcat(cmdArgs, " ");
    }
    strcat(cmdArgs, cmdArgsRaw[argc-3]);
  } else {
    cmdArgs = "";
  }
  printf("cmdArg=%s\n", cmdArgs);
  return 0;
  char *exec_argv[] = { "strace", "ls", NULL, };
  char *exec_envp[] = { "PATH=/bin", NULL, };
  execve("strace",          exec_argv, exec_envp);
  execve("/bin/strace",     exec_argv, exec_envp);
  execve("/usr/bin/strace", exec_argv, exec_envp);
  perror(argv[0]);
  exit(EXIT_FAILURE);
}
