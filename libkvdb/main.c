#include <stdio.h>
#include <assert.h>
#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include <stdlib.h>
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
  int fd;
  struct flock lock;
  struct dataList dataHeah;
};

struct kvdb *kvdb_open(const char *filename) {
    char workpath[1000];
    strcpy(workpath, "./");
    int fd = open(strcat(strcat(workpath, filename), ".db"), O_RDWR | O_CREAT);
    printf("Now is opening database:%s\n", workpath);
    close(fd);
    struct kvdb* pkvdb = malloc(sizeof(struct kvdb));
    if (pkvdb != NULL) {
        pkvdb->fd = fd;
    }
    return pkvdb;
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