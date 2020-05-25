#include <stdio.h>
#include <assert.h>
#include <stdio.h>
#include <fcntl.h>
struct data {
  void* dataSec;
  struct data* next;
};
struct kvdb {
  // your definition here
  FILE* fp;
  struct flock lock;
};

struct kvdb *kvdb_open(const char *filename) {
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
