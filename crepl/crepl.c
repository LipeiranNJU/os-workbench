#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <unistd.h>
#include <assert.h>
#include <dlfcn.h>
int (*mp)();
void* h;

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
  int fds[2];
  if(pipe(fds) < 0){
		perror("pipe");
    assert(0);
	}
  int ppid = fork();
  if (ppid == 0) {
    close(fds[0]);
    if (version == 32) {
      execvp("gcc", argv32);
    } else if (version == 64) {
      execvp("gcc", argv64);
    } else {
      assert(0);
    }
  } else {
    close(fds[1]);
    char ch = '\0';
    while (read(fds[0], &ch, 1)) ;
  }
  static char line[4096];
  while (1) {
    printf("crepl> ");
    fflush(stdout);
    if (!fgets(line, sizeof(line), stdin)) {
      break;
    }
    if (strlen(line) > 2) {
      if (strncmp(line, "int ", 3) == 0) {
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
          f1 = fopen("/tmp/abc.c", "r");
          f2 = fopen("/tmp/abc1.c", "w");
          while((c = fgetc(f1)) != EOF)
            fputc(c, f2);
          fclose(f1);
          fclose(f2);
          FILE *fp = fopen("/tmp/abc1.c","a");
          fprintf(fp, "%s", line);
          fprintf(fp, "\n");
          fclose(fp);
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
          close(pipefds[1]);
          char ch = '\0';
          while (read(pipefds[0], &ch, 1)) {
            if (ch != '\0') {
              printf("Compile Error!\n");
              status = 1;
              break;
            }
          }
          if (status == 0){
            int ffds[2];
            if(pipe(ffds) < 0){
		          perror("pipe");
              assert(0);
	          }
            int pppid = fork();
            if (pppid == 0) {
              close(ffds[0]);
              char* argv32[] = {"gcc", "-w", "-fPIC", "-shared", "-m32","/tmp/abc.c", "-o", "/tmp/abc.so", NULL};
              char* argv64[] = {"gcc", "-w", "-fPIC", "-shared", "-m64","/tmp/abc.c", "-o", "/tmp/abc.so", NULL};
              if (status == 0){
                FILE *fp = fopen("/tmp/abc.c","a");
                fprintf(fp, "%s", line);
                fprintf(fp, "\n");
                fclose(fp);
                if (version == 32) {
                  execvp("gcc", argv32);
                } else if (version == 64) {
                  execvp("gcc", argv64);
                } else {
                  assert(0);
                }
              }
            } else {
              close(ffds[1]);
              char chh = '\0';
              while (read(ffds[0], &ch, 1)) {};
              printf("add a function\n");
            }
          }

        }
        continue;
      }
    }
    remove("/tmp/wrapper.c");
    remove("/tmp/wrapper.so");
    int pipefds[2];
    if(pipe(pipefds) < 0){
		  perror("pipe");
      assert(0);
	  }

    int pid = fork();
    if (pid == 0) {
      close(pipefds[0]);
      dup2(pipefds[1], fileno(stderr));
      dup2(pipefds[1], fileno(stdout));
      FILE *f2;
      f2 = fopen("/tmp/wrapper1.c", "w");
      fclose(f2);
      FILE *fp = fopen("/tmp/wrapper1.c","w");
      fprintf(fp, "int __expr() { return (");
      fprintf(fp, "%s", line);
      fprintf(fp, ");}");
      fclose(fp);
      char* argv32[] = {"gcc", "-w", "-fPIC", "-shared", "-m32","/tmp/wrapper1.c", "/tmp/abc.so", "-o", "/tmp/wrapper1.so", NULL};
      char* argv64[] = {"gcc", "-w", "-fPIC", "-shared", "-m64","/tmp/wrapper1.c", "/tmp/abc.so", "-o", "/tmp/wrapper1.so", NULL};
      if (version == 32) {
        execvp("gcc", argv32);
      } else if (version == 64) {
        execvp("gcc", argv64);
      } else {
        assert(0);
      }
    } else {
      close(pipefds[1]);
      char chtmp = '\0';
      int status = 0;
      while (read(pipefds[0], &chtmp, 1)) {
        if (chtmp != '\0'){
          status = 1;
          break;
        }
      }
      if (status == 1) {
        printf("ex Compile Error!\n");
        continue;
      }
      FILE * f1;
      f1 = fopen("/tmp/wrapper.c", "w");
      fprintf(f1, "int __expr() { return (");
      fprintf(f1, "%s", line);
      fprintf(f1, ");}");
      fclose(f1);
      char* argv32[] = {"gcc", "-w", "-fPIC", "-shared", "-m32","/tmp/wrapper.c", "/tmp/abc.so", "-o", "/tmp/wrapper.so", NULL};
      char* argv64[] = {"gcc", "-w", "-fPIC", "-shared", "-m64","/tmp/wrapper.c", "/tmp/abc.so", "-o", "/tmp/wrapper.so", NULL};
      int fd[2];
      if(pipe(pipefds) < 0){
		    perror("pipe");
        assert(0);
	    }
      int pppid = fork();
      if (pppid == 0) {
        close(pipefds[0]);
        if (version == 32) {
          execvp("gcc", argv32);
        } else if (version == 64) {
          execvp("gcc", argv64);
        } else {
          assert(0);
        }
      } else {
        close(pipefds[1]);
        char chtmp = '\0';
        while (read(pipefds[0], &chtmp, 1)) ;
        h = dlopen("/tmp/wrapper.so", RTLD_NOW|RTLD_GLOBAL);
        mp = dlsym(h, "__expr");
        if (mp == NULL) {
          printf("Compile Error!\n");
        } else {
          assert(mp != NULL);
          printf("= %d\n", mp());
          dlclose(h);
        }
      }
    }
  }
  return 0;
}
