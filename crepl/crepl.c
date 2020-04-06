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
  remove("/tmp/wrapper.c");
  remove("/tmp/wrapper.so");
  remove("/tmp/abc.c");
  remove("/tmp/abc.so");
  FILE *fp = fopen("/tmp/abc.c","a");
  fclose(fp);
  int version = 0;
  int l = strlen(argv[0]);
  if (strncmp("32", &argv[0][l - 2], 2) == 0) {
    version = 32;
  } else if (strncmp("64", &argv[0][l - 2], 2) == 0){
    version = 64;
  } else {
    assert(0);
  }
  char* argv32[] = {"gcc", "-w", "-fPIC", "-shared", "-m32","/tmp/abc.c", "-o", "/tmp/abc.so", NULL};
  char* argv64[] = {"gcc", "-w", "-fPIC", "-shared", "-m64","/tmp/abc.c", "-o", "/tmp/abc.so", NULL};
  int ppid = fork();
  if (ppid == 0) {
    if (version == 32) {
      execvp("gcc", argv32);
    } else if (version == 64) {
      execvp("gcc", argv64);
    } else {
      assert(0);
    }
  }
  static char line[4096];
  while (1) {
    printf("crepl> ");
    fflush(stdout);
    if (!fgets(line, sizeof(line), stdin)) {
      break;
    }
    if (strncmp(line, "q", 1) == 0) {
      // break;
    }
    if (strlen(line) > 2) {
      if (strncmp(line, "int ", 3) == 0) {
        // printf("try to define a function\n");
        // printf("%s", line);
        FILE *fp = fopen("/tmp/abc.c","a");
        fprintf(fp, "%s", line);
        fprintf(fp, "\n");
        fclose(fp);
        int pipefds[2];
        if(pipe(pipefds) < 0){
		      perror("pipe");
          assert(0);
	      }
        int pid = fork();
        if (pid == 0){
          close(pipefds[0]);
          dup2(pipefds[1], fileno(stderr));
          dup2(pipefds[1], fileno(stdout));
          FILE *f1, *f2;
          int c;
          f1 = fopen("/tmp/abc.c", "w");
          f1 = fopen("/tmp/abc1.c", "w");
          while((c = fgetc(f1)) != EOF)
            fputc(c, f2);
          fclose(f1);
          fclose(f2);
          char* argv32[] = {"gcc", "-w", "-fPIC", "-shared", "-m32","/tmp/abc1.c", "-o", "/tmp/abc1.so", NULL};
          char* argv64[] = {"gcc", "-w", "-fPIC", "-shared", "-m64","/tmp/abc1.c", "-o", "/tmp/abc1.so", NULL};
          if (version == 32) {
            execvp("gcc", argv32);
          } else if (version == 64) {
            execvp("gcc", argv64);
          } else {
            assert(0);
          }
        } else {
          int status = 0;
          // printf("Hello\n");
          close(pipefds[1]);
          char ch = '\0';
          while (read(pipefds[0], &ch, 1)) {
            if (ch != '\0') {
              printf("Compile Error!\n");
              status = 1;
              break;
            }
          }
          char* argv32[] = {"gcc", "-w", "-fPIC", "-shared", "-m32","/tmp/abc.c", "-o", "/tmp/abc.so", NULL};
          char* argv64[] = {"gcc", "-w", "-fPIC", "-shared", "-m64","/tmp/abc.c", "-o", "/tmp/abc.so", NULL};
          if (status == 0){
            if (version == 32) {
              execvp("gcc", argv32);
            } else if (version == 64) {
              execvp("gcc", argv64);
            } else {
              assert(0);
            }
          }
        }
         
        continue;
      }
    }
    // printf("try to use an expression\n");
    remove("/tmp/wrapper.c");
    remove("/tmp/wrapper.so");
    FILE *fp = fopen("/tmp/wrapper.c","a");
    fprintf(fp, "int __expr() { return (");
    fprintf(fp, "%s", line);
    fprintf(fp, ");}");
    fclose(fp);
    int pid = fork();
    if (pid == 0) {
      char* argv32[] = {"gcc", "-w", "-fPIC", "-shared", "-m32","/tmp/wrapper.c", "/tmp/abc.so", "-o", "/tmp/wrapper.so", NULL};
      char* argv64[] = {"gcc", "-w", "-fPIC", "-shared", "-m64","/tmp/wrapper.c", "/tmp/abc.so", "-o", "/tmp/wrapper.so", NULL};
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
      h = dlopen("/tmp/wrapper.so", RTLD_NOW|RTLD_GLOBAL);
      mp = dlsym(h, "__expr");
      printf("%d\n", mp());
      dlclose(h);
      remove("/tmp/wrapper.c");
      remove("/tmp/wrapper.so");
    }
    continue;
    // printf("Got %zu chars.\n", strlen(line)); // WTF?
  }
  return 0;
}
