#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>

int main(int argc, char *argv[]) {
  char* cmd = argv[1];
  char** cmdArgs = &argv[2];
  printf("%s\n", cmd);
  for (int i = 0; i < argc-2; i++) {
    printf("%s\n", cmdArgs[2+i]);
  }
  return 0;
  char *exec_argv[] = { "strace", "ls", NULL, };
  char *exec_envp[] = { "PATH=/bin", NULL, };
  execve("strace",          exec_argv, exec_envp);
  execve("/bin/strace",     exec_argv, exec_envp);
  execve("/usr/bin/strace", exec_argv, exec_envp);
  perror(argv[0]);
  exit(EXIT_FAILURE);
}
