#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <unistd.h>

void defFunction(){

}
int main(int argc, char *argv[]) {
  static char line[4096];
  char template[] = "tmp-XXXXXX";
  while (1) {
    printf("crepl> ");
    fflush(stdout);
    if (!fgets(line, sizeof(line), stdin)) {
      break;
    }
    if (strncmp(line, "q", 1) == 0) {
      break;
    }
    if (strlen(line) > 2) {
      if (strncmp(line, "int ", 3) == 0) {
        printf("try to define a function\n");
        printf("%s", line);
        int fd = mkstemp(template);
        printf("fd:%d\n", fd);
        close(fd);
        continue;
      }
    }
    printf("try to use an expression\n");
    continue;
    // printf("Got %zu chars.\n", strlen(line)); // WTF?
  }
  // unlink(template);
  return 0;
}
