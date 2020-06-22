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
  char* name;
  struct flock lock;
  struct dataList dataHeah;
};

struct kvdb *kvdb_open(const char *filename) {
    char workpath[1000];
    strcpy(workpath, "./");
    int fd = open(strcat(strcat(workpath, filename), ".db"), O_RDWR | O_CREAT);
    lseek(fd, 0, SEEK_END);
    printf("Now is opening database:%s\n", workpath);
    struct kvdb* pkvdb = malloc(sizeof(struct kvdb));
    pkvdb->name = malloc(sizeof(char) * (strlen(workpath) + 1));
    strcpy(pkvdb->name, workpath);
    if (pkvdb != NULL) {
        pkvdb->fd = fd;
    }
    return pkvdb;
}

int kvdb_close(struct kvdb *db) {
    free(db->name);
    db->name =  NULL;
    return close(db->fd);
}

int kvdb_put(struct kvdb *db, const char *key, const char *value) {
    char* keyIdentifier = "@lprylkey";
    char* valueIdentifier = "^lprylvalue";
    char* endIdentifier = "$lprylend";
    write(db->fd, keyIdentifier, strlen(keyIdentifier));
    write(db->fd, key, strlen(key));
    write(db->fd, valueIdentifier, strlen(valueIdentifier));
    write(db->fd, value, strlen(value));
    write(db->fd, endIdentifier, strlen(endIdentifier));
    fsync(db->fd);
    return 0;
}

char *kvdb_get(struct kvdb *db, const char *key) {
  return NULL;
}

enum {QUIT, INSERT, DELETE, OPEN, CLOSE};

int main() {
    struct kvdb *db = NULL;
    int instruction = -1;
    while (instruction != QUIT) {
        printf("Please tell me what you want:\n");
        printf("[%d] QUIT\n", QUIT);
        printf("[%d] INSERT\n", INSERT);
        printf("[%d] DELETE\n", DELETE);
        printf("[%d] OPEN\n", OPEN);
        printf("[%d] CLOSE\n", CLOSE);
        scanf("%d", &instruction);
        if (instruction == OPEN) {
            printf("Please ENTER NAME:\n");
            char name[100];
            scanf("%s", name);
            printf("NAME:%s\n", name);
            db = kvdb_open(name);
            assert(db != NULL);
        } else if (instruction == CLOSE) {
            int closeStatus = kvdb_close(db);
            assert(closeStatus == 0);
            break;
        } else if (instruction == INSERT) {
            char key[100];
            char value[100];
            printf("Please ENTER KEY:\n");
            scanf("%s", key);
            printf("Please ENTER value:\n");
            scanf("%s", value);
            kvdb_put(db, key, value);
        }
        
    }

    // system("rm -f *.db");
    return 0;
}