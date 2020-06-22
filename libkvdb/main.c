#include <stdio.h>
#include <assert.h>
#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include <fcntl.h>
struct data {
  void* dataSec;
  struct data* next;
  void* key;
};
struct dataList {
  struct data* dataHead;
  struct dataList* next;
};
struct kvdb {
  // your definition here
  FILE* fp;
  struct flock lock;
  struct dataList dataHeah;
};

struct kvdb *kvdb_open(const char *filename) {
    char workpath[1000];
    strcpy(workpath, "./");
    FILE* fp = fopen(strcat(workpath, filename), "a+");
    printf("Now is opening database:%s\n", workpath);
    fclose(fp);
    return NULL;
}

int kvdb_close(struct kvdb *db) {
  return -1;
}

int kvdb_put(struct kvdb *db, const char *key, const char *value) {
  return -1;
}

char *kvdb_get(struct kvdb *db, const char *key) {
  return NULL;
}

enum {QUIT, INSERT, DELETE, CREATE};

int main() {
    int instruction = -1;
    while (instruction != QUIT) {
        printf("Please tell me what you want:\n");
        printf("[%d] QUIT\n", QUIT);
        printf("[%d] INSERT\n", INSERT);
        printf("[%d] DELETE\n", DELETE);
        printf("[%d] CREATE\n", CREATE);
        scanf("%d", &instruction);
        if (instruction == CREATE) {
            printf("Please ENTER NAME:\n");
            char name[100];
            scanf("%s", name);
            printf("NAME:%s\n", name);
            struct kvdb *openStatus = kvdb_open(name);
            assert(openStatus != NULL);
        }
    }
    return 0;
}