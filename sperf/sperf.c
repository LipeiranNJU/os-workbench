#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
int main(int argc, char *argv[]) {
  char** cmdArgs = malloc(sizeof(char*)*(argc + 2));
  cmdArgs[0] = "strace";
  cmdArgs[1] = "-T";
  for (int i = 1; i <= argc; i++){
    cmdArgs[i+1] = argv[i];
  }

  // char *exec_argv[] = { "strace", "ls", "-a", NULL, };
  char *exec_envp[] = { "PATH=/bin", NULL, };
  char *test[] = { "strace", "-T", "ls", NULL, };
  execve("/usr/bin/strace", cmdArgs, exec_envp);
  perror(argv[0]);
  exit(EXIT_FAILURE);
}
