#include <stdio.h>
#include <assert.h>
#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include <stdlib.h>
#include <fcntl.h>
#include <sys/stat.h>
#include <stdint.h>
#include <stdbool.h>

#define USUALLINEWIDTH 4242
#define B 1
#define KB (1024 * B)
#define MB (1024 * KB)
#define KEYSIZE (128 * B)
#define USUALVALUESIZE (4 * KB)
#define MAXVALUESIZE (16 * MB)
enum {UPDATE, INSERT};

struct record {
  char* key;
  char* value;
  struct record *next;
  int64_t lineindex;
  bool valid;
};

struct kvdb {
  // your definition here
  int fd;
  struct record *recordListHead;
};
char workpath[1000];
enum {U, M};
struct kvdb *kvdb_open(const char *filename) {
    strcpy(workpath, "/tmp/");
    int fd = open(strcat(strcat(workpath, filename), ".db"), O_RDWR | O_CREAT, 0777);
    long offset = 0;
    struct stat statbuf;  
    stat(workpath, &statbuf);  
    int fileSize = statbuf.st_size;  

    struct kvdb* pkvdb = malloc(sizeof(struct kvdb));
    pkvdb->recordListHead = malloc(sizeof(struct record));
    pkvdb->recordListHead->next = NULL;
    pkvdb->recordListHead->key = malloc(sizeof(char));
    pkvdb->recordListHead->value = malloc(sizeof(char));
    pkvdb->recordListHead->key = '\0';
    pkvdb->recordListHead->value = '\0';
    pkvdb->recordListHead->lineindex = -1;
    // assert(0);
    pkvdb->fd = fd;
    system("pwd");
    lseek(fd, 0, SEEK_SET);
    if (fileSize > 0) {
        struct record* tmptail;
        for (tmptail = pkvdb->recordListHead; tmptail->next != NULL; tmptail = tmptail->next) ;
        while (offset < fileSize) {
            struct record* tmprecord = malloc(sizeof(struct record));
            tmprecord->valid = true;
            tmprecord->next = NULL;
            tmprecord->lineindex = tmptail->lineindex+1;
            tmptail->next = tmprecord;
            tmptail = tmptail->next;
            char sizeString[9];
            char **tmp = NULL;
            memset(sizeString, '\0', 9);

            char ch;
            int linemode;
            read(pkvdb->fd, &ch, 1);
            if (ch == 'U') {
                linemode = U;
            } else if (ch == 'M') {
                linemode = M;
            } else {
                assert(0);
            }
            
            read(pkvdb->fd, sizeString, 8);
            offset += 8;
            long keySize = strtol(sizeString, tmp, 16);
            tmprecord->key = malloc(sizeof(keySize)+1);
            tmprecord->key[keySize] = '\0';
            read(pkvdb->fd, tmprecord->key, keySize);
            lseek(pkvdb->fd, KEYSIZE-keySize, SEEK_CUR);
            offset += KEYSIZE;
            if (linemode == U) {
                read(pkvdb->fd, sizeString, 8);
                offset += 8;
                long valueSize = strtol(sizeString, tmp, 16);
                tmprecord->value = malloc(sizeof(valueSize)+1);
                tmprecord->value[valueSize] = '\0';
                read(pkvdb->fd, tmprecord->value, valueSize);
                lseek(pkvdb->fd, USUALVALUESIZE-valueSize+1, SEEK_CUR);
                offset += USUALVALUESIZE + 1;
            } else if (linemode == M) {
                read(pkvdb->fd, sizeString, 8);
                offset += 8;
                long valueSize = strtol(sizeString, tmp, 16);
                tmprecord->value = malloc(sizeof(valueSize)+1);
                tmprecord->value[valueSize] = '\0';
                read(pkvdb->fd, tmprecord->value, valueSize);
                lseek(pkvdb->fd, MAXVALUESIZE-valueSize+1, SEEK_CUR);
                offset += MAXVALUESIZE + 1;
            } else {
                assert(0);
            }

        }

    } else {
        ;
    }
    
    lseek(fd, 0, SEEK_END);
    return pkvdb;
}

int kvdb_close(struct kvdb *db) {
    struct record *tmp = db->recordListHead;
    while (tmp != NULL) {
        if (tmp->key != NULL) {
            free(tmp->key);
            tmp->key = NULL;
        }
        tmp = tmp->next;
    } 
    close(db->fd);
    free(db);
    return 0;
}

int kvdb_put(struct kvdb *db, const char *key, const char *value) {
    // update db in memory
    struct record *tmp;
    int status = INSERT;
    int64_t lineindex = -1;
    for (tmp = db->recordListHead; tmp != NULL; tmp = tmp->next) {
        if (tmp->key == NULL) 
            continue;
        if (strcmp(key, tmp->key) == 0) {
            free(tmp->value);
            status = UPDATE;
            tmp->value = malloc(sizeof(char) * (strlen(value) + 1));
            lineindex = tmp->lineindex;
            strcpy(tmp->value, value);
        }
    }
    if (status == INSERT) {
        struct record* tmptail;
        for (tmptail = db->recordListHead; tmptail->next != NULL; tmptail = tmptail->next);
        struct record* tmprecord = malloc(sizeof(struct record));
        tmprecord->lineindex = tmptail->lineindex+1;
        tmprecord->next = NULL;
        tmptail->next = tmprecord;
        tmptail = tmptail->next;
        tmprecord->key = malloc(sizeof(char) * (strlen(key)+1));
        tmprecord->value = malloc(sizeof(char) * (strlen(value)+1));
        strcpy(tmprecord->key, key);
        strcpy(tmprecord->value, value);
    }

    if (status == INSERT) {
        lseek(db->fd, 0, SEEK_END);
    } else {
        lseek(db->fd, lineindex*USUALLINEWIDTH, SEEK_SET);
    }
    int linemode;
    if (strlen(value) <= USUALVALUESIZE) {
        linemode = U;
    } else {
        linemode = M;
    }
    if (linemode == U) {
        write(db->fd, "U", 1);
    } else if (linemode == M) {
        write(db->fd, "M", 1);
    } else {
        assert(0);
    }
    int keysizeused = strlen(key);
    int valuesizeused = strlen(value);
    assert(valuesizeused < USUALVALUESIZE);
    char keysizestring[9];
    char valuesizestring[9];
    memset(keysizestring, '\0', 9);
    memset(valuesizestring, '\0', 9);
    sprintf(keysizestring, "%08x", keysizeused);
    sprintf(valuesizestring, "%08x", valuesizeused);

    write(db->fd, keysizestring, 8);
    int compensate = KEYSIZE - strlen(key);
    char* spaces = malloc(sizeof(char) * compensate);
    memset(spaces, ' ', compensate);
    write(db->fd, key, strlen(key));
    write(db->fd, spaces, compensate);
    free(spaces);
    spaces = NULL;

    compensate = USUALVALUESIZE - strlen(value);
    spaces = malloc(sizeof(char) * compensate);
    memset(spaces, ' ', compensate);
    write(db->fd, valuesizestring, 8);
    write(db->fd, value, strlen(value));
    write(db->fd, spaces, compensate);
    write(db->fd, "\n", 1);
    fsync(db->fd);
    return 0;
}

char *kvdb_get(struct kvdb *db, const char *key) {
    struct record *tmp;
    for (tmp = db->recordListHead; tmp != NULL; tmp = tmp->next) {
        if (tmp->key == NULL) 
            continue;
        if (strcmp(key, tmp->key) == 0) 
            return tmp->value;
    }
    return NULL;
}
