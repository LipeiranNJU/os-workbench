#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <unistd.h>
#include <assert.h>
#include <dlfcn.h>
int (*mp)();
void* h;
void defFunction(){

}
int main(int argc, char *argv[]) {
  int version = 0;
  int l = strlen(argv[0]);
  if (strncmp("32", &argv[0][l - 2], 2) == 0) {
    version = 32;
  } else if (strncmp("64", &argv[0][l - 2], 2) == 0){
    version = 64;
  } else {
    assert(0);
  }
  static char line[4096];
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
        // printf("try to define a function\n");
        printf("%s", line);
        FILE *fp = fopen("/tmp/abc.c","w+");
        fprintf(fp, "%s", line);
        fclose(fp);
        int pid = fork();
        if (pid == 0){
          char* argv32[] = {"gcc", "-w", "-fPIC", "-shared", "-m32","/tmp/abc.c", "-o", "abc.so", NULL};
          char* argv64[] = {"gcc", "-w", "-fPIC", "-shared", "-m64","/tmp/abc.c", "-o", "abc.so", NULL};
          if (version == 32) {
            execvp("gcc", argv32);
          } else if (version == 64) {
            execvp("gcc", argv64);
          } else {
            assert(0);
          }
        }
        continue;
      }
    }
    // printf("try to use an expression\n");
    FILE *fp = fopen("wrapper.c","a");
    fprintf(fp, "int __expr() { return (");
    fprintf(fp, "%s", line);
    fprintf(fp, ");}");
    fclose(fp);
    int pid = fork();
    if (pid == 0) {
      char* argv32[] = {"gcc", "-w", "-fPIC", "-shared", "-m32","wrapper.c", "abc.so", "-o", "wrapper.so", NULL};
      char* argv64[] = {"gcc", "-w", "-fPIC", "-shared", "-m64","wrapper.c", "abc.so", "-o", "wrapper.so", NULL};
      if (version == 32) {
        execvp("gcc", argv32);
      } else if (version == 64) {
        execvp("gcc", argv64);
      } else {
        assert(0);
      }
    }
    if (pid != 0) {
      sleep(1);
      h = dlopen("./wrapper.so", RTLD_NOW|RTLD_GLOBAL);
      mp = dlsym(h, "__expr");
      printf("%d\n", mp());
      dlclose(h);
      remove("./wrapper.c");
      remove("./wrapper.so");
    }
    continue;
    // printf("Got %zu chars.\n", strlen(line)); // WTF?
  }
  return 0;
}
