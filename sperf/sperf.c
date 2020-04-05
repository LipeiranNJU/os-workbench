#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
int main(int argc, char *argv[]) {
  char *exec_argv[] = { "strace", "ls", "-a", NULL, };
  char *exec_envp[] = { "PATH=/bin", NULL, };
  char *test[] = { "strace", "-T", "ls", NULL, };
  execve("/usr/bin/strace", test, exec_envp);
  perror(argv[0]);
  exit(EXIT_FAILURE);
}
