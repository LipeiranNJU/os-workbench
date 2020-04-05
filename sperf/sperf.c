#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
int main(int argc, char *argv[]) {
  char* cmd = argv[1];
  char ** cmdArgsRaw = NULL;
  char* cmdArgs = NULL;
  // if (argc >= 2) {
  //   cmdArgsRaw = &argv[2];
  //   printf("cmd=%s\n", cmd);
  // }
  // if (argc > 2) {
  //   int len = 0;
  //   for (int i = 0; i < argc-2; i++) {
  //     len += strlen(cmdArgsRaw[i]);
  //     len += 1;
  //   }
  //   cmdArgs = malloc(len);
  //   memset(cmdArgs, '\0', len);
  //   for (int i = 0; i < argc-3; i++) {
  //     strcat(cmdArgs, cmdArgsRaw[i]);
  //     strcat(cmdArgs, " ");
  //   }
  //   strcat(cmdArgs, cmdArgsRaw[argc-3]);
  // } else {
  //   cmdArgs = "";
  // }
  printf("cmdArg=%s\n", cmdArgs);
  char *exec_argv[] = { "strace", "ls", "-a", "-l", "> /dev/null", NULL, };
  char *exec_envp[] = { "PATH=/bin", NULL, };
  execve("/usr/bin/strace", &argv[2], exec_envp);
  perror(argv[0]);
  exit(EXIT_FAILURE);
}
