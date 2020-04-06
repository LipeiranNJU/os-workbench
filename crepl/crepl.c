#include <stdio.h>
#include <string.h>
void defFunction(){

}
int main(int argc, char *argv[]) {
  static char line[4096];
  while (1) {
    printf("crepl> ");
    fflush(stdout);
    if (!fgets(line, sizeof(line), stdin)) {
      break;
    }
    if (strlen(line) > 3) {
      if (strncmp(line, "int ", 4) == 0) {
        printf("define a function");
        continue;
      }
    }
    printf("Got %zu chars.\n", strlen(line)); // WTF?
  }
}
