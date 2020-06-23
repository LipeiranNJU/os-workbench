#include <stdio.h>
#include <assert.h>
#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include <stdlib.h>
#include <fcntl.h>

char* keyIdentifier = "@lprylkey";
char* valueIdentifier = "^lprylvalue";
char* endIdentifier = "$lprylend";

// struct data {
//   void* dataSec;
//   struct data* next;
//   void* key;
// };
struct record {
  void* key;
  void* value;
};
// struct dataList {
//   struct data* dataHead;
//   struct dataList* next;
// };
struct kvdb {
  // your definition here
  int fd;
//   struct flock lock;
  struct record* recordHead;
};

struct kvdb *kvdb_open(const char *filename) {
    char workpath[1000];
    strcpy(workpath, "./");
    int fd = open(strcat(strcat(workpath, filename), ".db"), O_RDWR | O_CREAT, 0777);
    lseek(fd, 0, SEEK_END);
    
    printf("Now is opening database:%s\n", workpath);
    struct kvdb* pkvdb = malloc(sizeof(struct kvdb));
    if (pkvdb != NULL) {
        pkvdb->fd = fd;
    }
    lseek(fd, 0, SEEK_SET);
    char ch;
    
    do {
        ch = read(pkvdb->fd, &ch, 1);
    } while(ch != EOF);
    
    assert(0);
    lseek(fd, 0, SEEK_END);
    assert(0);
    return pkvdb;
}

int kvdb_close(struct kvdb *db) {
    return close(db->fd);
}

int kvdb_put(struct kvdb *db, const char *key, const char *value) {
    write(db->fd, keyIdentifier, strlen(keyIdentifier));
    write(db->fd, key, strlen(key));
    int keysize = strlen(key);
    char keysizestring[9];
    memset(keysizestring, '\0', 9);
    sprintf(keysizestring, "%0x", keysize);
    write(db->fd, valueIdentifier, strlen(valueIdentifier));
    write(db->fd, keysizestring, 8);
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