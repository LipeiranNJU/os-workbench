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

#define B 1
#define KB (1024 * B)
#define MB (1024 * KB)
#define JOURNALSIZE 10
#define KEYSIZE (1 + 128 * B + 9 + 9 + 9)
#define KEYITEMS (50)
#define KEYAREASIZE (KEYSIZE * KEYITEMS)


enum {UPDATE, INSERT};

struct record {
    char valid;
    char KEY[128];
    char keysize[9];
    char valuesize[9];
    char clusnum[9];
};

struct kvdb {
  // your definition here
  int fd;
  struct record *database;
};
char workpath[1000];
enum {U, M, H, S};

void goto_journal(const struct kvdb *db) {
    lseek(db->fd, 0, SEEK_SET);
}

void goto_keyarea(const struct kvdb *db) {
    lseek(db->fd, JOURNALSIZE, SEEK_SET);
}

void goto_clusters(const struct kvdb *db) {
    lseek(db->fd, (JOURNALSIZE+KEYAREASIZE), SEEK_SET);
}

void load_database(struct kvdb* db) {
    db->database= malloc(sizeof(char)*KEYAREASIZE);
    lseek(db->fd, JOURNALSIZE, SEEK_SET);
    read(db->fd, db->database, KEYAREASIZE);
    return ;
}

void setkeyondisk(struct kvdb* db,const char* keyname, const char* valuename, const int keyindex_i) {
    int keySizeNum = strlen(keyname);
    int valueSizeNum = strlen(valuename);
    char keysize[8];
    char valuesize[8];
    sprintf(keysize, "%08x", keySizeNum);
    sprintf(valuesize, "%08x", valueSizeNum);
    int end = lseek(db->fd, 0, SEEK_END);
    int clusnum = (end-JOURNALSIZE-KEYAREASIZE)/(4*KB);
    printf("offset:%d\n",(end-JOURNALSIZE-KEYSIZE));
    printf("clusnum:%d\n", clusnum);
    lseek(db->fd, JOURNALSIZE+keyindex_i*KEYSIZE, SEEK_SET);
    struct record* keybuffer = malloc(sizeof(char)*KEYSIZE);
    sprintf(keybuffer->clusnum, "%08x", clusnum);
    keybuffer->valid = '1';
    memcpy(keybuffer->keysize, keysize, 8);
    memcpy(keybuffer->valuesize, valuesize, 8);
    memcpy(keybuffer->KEY, keyname, keySizeNum);
    printf("keysize:%s\n", keysize);
    write(db->fd, keybuffer, KEYSIZE);
    fsync(db->fd);
    free(keybuffer);
    keybuffer = NULL;
}


void unload_database(struct kvdb *db) {
    free(db->database);
    db->database = NULL;
    return ;
}
struct kvdb *kvdb_open(const char *filename) {
    strcpy(workpath, "/home/lpr/os-workbench/libkvdb/");
    strcat(strcat(workpath, filename), ".db");
    int fd = open(workpath, O_RDWR | O_CREAT, 0777);
    // long offset = 0;
    struct stat statbuf;  
    stat(workpath, &statbuf);  
    struct kvdb* pkvdb = malloc(sizeof(struct kvdb));
    pkvdb->fd = fd;
    int fileSize = statbuf.st_size;  
    printf("filesize:%d\n", fileSize);
    if (fileSize == 0) {
        char* zeros = malloc(sizeof(char)*(16*MB+1*KB));
        memset(zeros, '0', JOURNALSIZE);
        write(pkvdb->fd, zeros, JOURNALSIZE);
        fsync(pkvdb->fd);
        free(zeros);
        zeros = NULL;

        goto_keyarea(pkvdb);
        zeros = malloc(sizeof(char)*KEYAREASIZE);
        memset(zeros, '0', KEYAREASIZE);
        write(pkvdb->fd, zeros, KEYAREASIZE);
        fsync(pkvdb->fd);
        free(zeros);
        zeros = NULL;
    }
    printf("Now is opening database:%s\n", workpath);
    memset(workpath, '\0', 1000);
    return pkvdb;
}

int kvdb_close(struct kvdb *db) {

    close(db->fd);
    free(db);
    db = NULL;
    return 0;
}

int kvdb_put(struct kvdb *db, const char *key, const char *value) {
    load_database(db);
    int i;
    int valuelength = strlen(value);
    for (i = 0; db->database[i].valid != '0'; i++) {
        printf("NAME:%s\n", db->database[i].KEY);
        if (strcmp(db->database[i].KEY, key) == 0) {
            break;
        }
    }
    printf("key:%s\ti:%d\n", key, i);

    if (db->database[i].valid == '0') {
        setkeyondisk(db, key, value, i);
        lseek(db->fd, 0, SEEK_END);
        if (valuelength <= 4*KB) {
            char* valuebuffer = malloc(sizeof(char) * 4 * KB);
            memcpy(valuebuffer, value, valuelength);
            write(db->fd, valuebuffer, 4*KB);
            fsync(db->fd);
            free(valuebuffer);
            valuebuffer = NULL;
        } else {
            char* valuebuffer = malloc(sizeof(char) * 16 * MB);
            memcpy(valuebuffer, value, valuelength);
            write(db->fd, valuebuffer, 16 * MB);
            fsync(db->fd);
            free(valuebuffer);
            valuebuffer = NULL;
        }
    } else {
        int rawsize = strtol(db->database[i].keysize, NULL, 16);
        if (rawsize < 4*KB && valuelength >=4*KB) {
            setkeyondisk(db, key, value, i);
            lseek(db->fd, 0, SEEK_END);
            char* valuebuffer = malloc(sizeof(char) * 16 * MB);
            memcpy(valuebuffer, value, valuelength);
            write(db->fd, valuebuffer, 16 * MB);
            fsync(db->fd);
            free(valuebuffer);
            valuebuffer = NULL;
        } else {
            int keySizeNum = strlen(key);
            int valueSizeNum = strlen(value);
            char keysize[9];
            char valuesize[9];
            sprintf(keysize, "%08x", keySizeNum);
            sprintf(valuesize, "%08x", valueSizeNum);
            lseek(db->fd, JOURNALSIZE+i*KEYSIZE, SEEK_SET);
            struct record* keybuffer = malloc(sizeof(char)*KEYSIZE);
            keybuffer->valid = '1';
            memcpy(keybuffer->keysize, keysize, 8);
            memcpy(keybuffer->valuesize, valuesize, 8);
            memcpy(keybuffer->KEY, key, keySizeNum);
            memcpy(keybuffer->clusnum, db->database[i].clusnum, 8);
            printf("keysize:%s\n", keysize);
            write(db->fd, keybuffer, KEYSIZE);
            fsync(db->fd);
            free(keybuffer);
            keybuffer = NULL;
            int clusindex = strtol(db->database[i].clusnum, NULL, 16);
            lseek(db->fd, JOURNALSIZE+KEYAREASIZE+clusindex*4*KB, SEEK_SET);
            write(db->fd, value, valuelength);
        }
    }
    unload_database(db);
    return 0;
}

char *kvdb_get(struct kvdb *db, const char *key) {
    load_database(db);
    int clusNum = -1;
    int valueSize = -1;
    char* returned = NULL;
    for (int i = 0; db->database[i].valid != '0'; i++) {
        if (strncmp(db->database[i].KEY, key, strtol(db->database[i].keysize, NULL, 16)) == 0) {
            clusNum = strtol(db->database[i].clusnum, NULL, 16);
            valueSize = strtol(db->database[i].valuesize, NULL, 16);
            break;
        }
    }
    if (clusNum > 0 && valueSize >0) {
        returned = malloc(sizeof(char)*(valueSize+1));
        returned[valueSize] = '\0';
        lseek(db->fd, JOURNALSIZE+KEYAREASIZE+clusNum*4*KB,SEEK_SET);
        read(db->fd, returned, valueSize);
    }
    unload_database(db);
    return returned;
}