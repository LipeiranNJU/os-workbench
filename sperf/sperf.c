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
  char *pathvar = getenv("PATH"); 
  char* path = malloc(sizeof(char)*(strlen("PATH=") + strlen(pathvar)+1));
  memset(path, '\0', strlen("PATH=") + strlen(pathvar)+1);
  strcat(path, "PATH=");
  strcat(path, pathvar);
  // char *exec_argv[] = { "strace", "ls", "-a", NULL, };
  char *exec_envp[] = { path, NULL, };
  char *test[] = { "strace", "-T", "ls", NULL, };
  execve("/usr/bin/strace", cmdArgs, exec_envp);
  perror(argv[0]);
  exit(EXIT_FAILURE);
}
