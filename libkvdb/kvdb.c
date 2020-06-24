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
enum {U, M, H, S};

void load_database(const struct kvdb *db) {
    lseek(db->fd, 0, SEEK_SET);
    struct record* tmptail;
    for (tmptail = db->recordListHead; tmptail->next != NULL; tmptail = tmptail->next);
    while (true) {
        struct record* tmprecord = malloc(sizeof(struct record));
        char sizeString[9];
        char **tmp = NULL;
        memset(sizeString, '\0', 9);
        char ch;
        int linemode;
        read(db->fd, &ch, 1);
        if (ch == 'U') {
            linemode = U;
        } else if (ch == 'M') {
            linemode = M;
        } else if (ch == 'H') {
            linemode = H;
        } else if (ch == ' ') {
            linemode = S;
        } else {
            close(db->fd);
            exit(3);
        }

        if (linemode == U) {
                tmprecord->valid = true;
                tmprecord->next = NULL;
                tmprecord->lineindex = tmptail->lineindex+1;
                tmptail->next = tmprecord;
                tmptail = tmptail->next;
                read(db->fd, sizeString, 8);
                long keySize = strtol(sizeString, tmp, 16);
                tmprecord->key = malloc(sizeof(keySize)+1);
                tmprecord->key[keySize] = '\0';
                read(db->fd, tmprecord->key, keySize);
                lseek(db->fd, KEYSIZE-keySize, SEEK_CUR);
                read(db->fd, sizeString, 8);
                long valueSize = strtol(sizeString, tmp, 16);
                tmprecord->value = malloc(sizeof(valueSize)+1);
                tmprecord->value[valueSize] = '\0';
                read(db->fd, tmprecord->value, valueSize);
                lseek(db->fd, USUALVALUESIZE-valueSize+1, SEEK_CUR);
            } else if (linemode == M) {
                tmprecord->valid = true;
                tmprecord->next = NULL;
                tmprecord->lineindex = tmptail->lineindex+1;
                tmptail->next = tmprecord;
                tmptail = tmptail->next;
                read(db->fd, sizeString, 8);
                long keySize = strtol(sizeString, tmp, 16);

                tmprecord->key = malloc(sizeof(keySize)+1);
                tmprecord->key[keySize] = '\0';
                read(db->fd, tmprecord->key, keySize);
                lseek(db->fd, KEYSIZE-keySize, SEEK_CUR);

                read(db->fd, sizeString, 8);
                long valueSize = strtol(sizeString, tmp, 16);

                tmprecord->value = malloc(sizeof(valueSize)+1);
                tmprecord->value[valueSize] = '\0';
                read(db->fd, tmprecord->value, valueSize);
                lseek(db->fd, MAXVALUESIZE-valueSize+1, SEEK_CUR);
            } else if (linemode == H) {
                free(tmprecord);
                tmprecord = NULL;
                lseek(db->fd, USUALLINEWIDTH-1, SEEK_CUR);
            } else if (linemode == S) {
                free(tmprecord);
                tmprecord = NULL;
                break;
            } else {
                assert(0);
            }

        
    }
    

    return ;
}
void unload_database(const struct kvdb *db) {
    struct record* firstrecord = db->recordListHead->next;
    if (firstrecord != NULL) {
        db->recordListHead->next = NULL;
        for (struct record* tmprecord = firstrecord; tmprecord != NULL;) {
            tmprecord = tmprecord->next;
            free(firstrecord);
            firstrecord = NULL;
            firstrecord = tmprecord;
        }
    }

}
struct kvdb *kvdb_open(const char *filename) {
    strcpy(workpath, "/home/lpr/os-workbench/libkvdb/");
    strcat(strcat(workpath, filename), ".db");
    int fd = open(workpath, O_RDWR | O_CREAT, 0777);
    // long offset = 0;
    struct stat statbuf;  
    stat(workpath, &statbuf);  
    int fileSize = statbuf.st_size;  

    if (fileSize == 0) {
        char* zeros = malloc(sizeof(char)*USUALLINEWIDTH*1024);
        memset(zeros, ' ', USUALLINEWIDTH*1024);
        for (int i = 1; i <= 1024; i++) {
            zeros[i*USUALLINEWIDTH-1] = '\n';
        }
        write(fd, zeros, USUALLINEWIDTH*1024);
        lseek(fd, 0, SEEK_SET);
        fsync(fd);

        char* tmpstring = malloc(sizeof(char)*USUALLINEWIDTH);
        memset(tmpstring, ' ', USUALLINEWIDTH);
        write(fd, "H", 1);
        write(fd, tmpstring, USUALLINEWIDTH-2);
        write(fd, "\n", 1);
        lseek(fd, 0, SEEK_SET);
        fsync(fd);
    }

    struct kvdb* pkvdb = malloc(sizeof(struct kvdb));
    pkvdb->recordListHead = malloc(sizeof(struct record));
    pkvdb->recordListHead->next = NULL;
    pkvdb->recordListHead->key = malloc(sizeof(char));
    pkvdb->recordListHead->value = malloc(sizeof(char));
    pkvdb->recordListHead->key[0] = '\0';
    pkvdb->recordListHead->value[0] = '\0';
    pkvdb->recordListHead->lineindex = 0;
    pkvdb->fd = fd;
    system("pwd");
    lseek(fd, 0, SEEK_SET);

    load_database(pkvdb);
    
    lseek(fd, 0, SEEK_END);
    memset(workpath, '\0', 1000);
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
    db = NULL;
    return 0;
}

int kvdb_put(struct kvdb *db, const char *key, const char *value) {
    // update db in memory
    load_database(db);
    struct record *tmp;
    int status = INSERT;
    int64_t lineindex = -1;
    int ii = 0;
    int64_t lastlineindex = -1;
    for (tmp = db->recordListHead; tmp != NULL; tmp = tmp->next) {

        if (strcmp(key, tmp->key) == 0) {
            free(tmp->value);
            tmp->value = NULL;
            status = UPDATE;
            // tmp->value = malloc(sizeof(char) * (strlen(value) + 1));
            lineindex = tmp->lineindex;
            // strcpy(tmp->value, value);
        } else if (strcmp("", tmp->key) != 0) {
            free(tmp->value);
            tmp->value = NULL;
        }
        if (tmp->next == NULL) {
            lastlineindex = tmp->lineindex;
        }

    }

    if (status == INSERT) {
        lseek(db->fd, (lastlineindex+1)*USUALLINEWIDTH, SEEK_SET);
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
    unload_database(db);
    return 0;
}

char *kvdb_get(struct kvdb *db, const char *key) {
    load_database(db);
    struct record *tmp;
    char* valuereturned = NULL;
    for (tmp = db->recordListHead; tmp != NULL; tmp = tmp->next) {
        if (strcmp(key, tmp->key) == 0) {
            valuereturned = malloc(sizeof(char)*(strlen(tmp->value)+1));
            strcpy(valuereturned, tmp->value);
        }
    }
    unload_database(db);
    return valuereturned;
}
