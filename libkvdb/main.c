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
#include <time.h>
#define B 1
#define KB (1024 * B)
#define MB (1024 * KB)
#define JOURNALSIZE 10
#define KEYSIZE (1 + 128 * B + 9 + 9 + 9 + 1)
#define KEYITEMS (1024)
#define KEYAREASIZE (KEYSIZE * KEYITEMS)
#define LONGVALUE (16*MB)
#define SHORTVALUE (4*KB)




struct record {
    char valid;
    char KEY[128];
    char keysize[9];
    char valuesize[9];
    char clusnum[9];
    char isLong;
};

struct kvdb {
  // your definition here
  int fd;
  struct record *database;
  int filesize;
};
char workpath[1000];

void load_database(struct kvdb* db) {
    db->filesize = lseek(db->fd, 0, SEEK_END);
    struct kvdb* pkvdb = malloc(sizeof(struct kvdb));
    db->database= malloc(sizeof(char)*KEYAREASIZE);
    lseek(db->fd, JOURNALSIZE, SEEK_SET);
    read(db->fd, db->database, KEYAREASIZE);
    return ;
}

void setkeyondisk(struct kvdb* db,const char* keyname, const char* valuename, const int keyindex_i) {
    int keySizeNum = strlen(keyname);
    int valueSizeNum = strlen(valuename);
    char keysize[9];
    char valuesize[9];
    sprintf(keysize, "%07x", keySizeNum);
    sprintf(valuesize, "%07x", valueSizeNum);
    int end = lseek(db->fd, 0, SEEK_END);
    int clusnum = (end-JOURNALSIZE-KEYAREASIZE)/(4*KB);


    struct record* keybuffer = malloc(sizeof(char)*KEYSIZE);
    sprintf(keybuffer->clusnum, "%07x", clusnum);
    sprintf(db->database[keyindex_i].clusnum, "%07x", clusnum);
    keybuffer->valid = 1;
    memcpy(keybuffer->keysize, keysize, 9);
    memcpy(keybuffer->valuesize, valuesize, 9);
    memcpy(keybuffer->KEY, keyname, keySizeNum);


    if (valueSizeNum > 4*KB) {
        keybuffer->isLong = 1;
    } else {
        keybuffer->isLong = 0;
    }
    lseek(db->fd, JOURNALSIZE+keyindex_i*KEYSIZE, SEEK_SET);
    write(db->fd, keybuffer, KEYSIZE);
    free(keybuffer);
    keybuffer = NULL;
}


void unload_database(struct kvdb *db) {
    free(db->database);
    db->database = NULL;
    return ;
}
struct kvdb *kvdb_open(const char *filename) {
    int fd = open(filename, O_RDWR | O_CREAT, 0777);
    // long offset = 0;
    struct stat statbuf;  
    stat(filename, &statbuf);  
    struct kvdb* pkvdb = malloc(sizeof(struct kvdb));
    pkvdb->fd = fd;
    int fileSize = statbuf.st_size;  


    if (fileSize == 0) {
        char* zeros = malloc(JOURNALSIZE);
        memset(zeros, 0, JOURNALSIZE);
        write(pkvdb->fd, zeros, JOURNALSIZE);
        free(zeros);
        zeros = NULL;

        zeros = malloc(sizeof(char)*KEYAREASIZE);
        memset(zeros, 0, KEYAREASIZE);
        write(pkvdb->fd, zeros, KEYAREASIZE);

        fsync(pkvdb->fd);

    }


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
    for (i = 0; db->database[i].valid != 0; i++) {
        int len = strtol(db->database[i].keysize, NULL, 16);
        char* keystored = malloc(len+1);
        keystored[len] = '\0';
        memcpy(keystored, db->database[i].KEY, len);
        if (strcmp(keystored, key) == 0) {
            break;
        }
    }


    if (db->database[i].valid == 0) {
        setkeyondisk(db, key, value, i);
        lseek(db->fd, 0, SEEK_END);
        if (valuelength <= 4*KB) {
            lseek(db->fd, 0, SEEK_END);
            write(db->fd, value, valuelength);
            if (4*KB-valuelength-1 > 0) {
                lseek(db->fd, 4*KB-valuelength-1, SEEK_CUR);
                write(db->fd, "0", 1);
            }

        } else {
            lseek(db->fd, 0, SEEK_END);
            write(db->fd, value, valuelength);
            if (16*MB-valuelength-1 > 0) {
                lseek(db->fd, 16*MB-valuelength-1, SEEK_CUR);
                write(db->fd, "0", 1);
            }
        }
    } else {
        if (db->database[i].isLong == 0 && valuelength >= 4*KB) {
            setkeyondisk(db, key, value, i);
            int end = lseek(db->fd, 0, SEEK_END);
            int clus = strtol(db->database[i].clusnum, NULL, 16);
            assert(end == clus*4*KB+JOURNALSIZE+KEYAREASIZE);
            // write(db->fd, valuebuffer, 16 * MB);
            lseek(db->fd, 0, SEEK_END);
            write(db->fd, value, valuelength);
            if (16*MB-valuelength-1 > 0) {
                lseek(db->fd, 16*MB-valuelength-1, SEEK_CUR);
                write(db->fd, "0", 1);
            }
        } else {
            int keySizeNum = strlen(key);
            int valueSizeNum = strlen(value);
            char keysize[9];
            char valuesize[9];
            sprintf(keysize, "%07x", keySizeNum);
            sprintf(valuesize, "%07x", valueSizeNum);
            lseek(db->fd, JOURNALSIZE+i*KEYSIZE, SEEK_SET);
            struct record* keybuffer = malloc(sizeof(char)*KEYSIZE);
            keybuffer->valid = 1;
            memcpy(keybuffer->keysize, keysize, 9);
            memcpy(keybuffer->valuesize, valuesize, 9);
            memcpy(keybuffer->KEY, key, keySizeNum);
            memcpy(keybuffer->clusnum, db->database[i].clusnum, 9);


            int clusindex = strtol(db->database[i].clusnum, NULL, 16);
            int isLong;
            if (db->database[i].isLong == 0) {
                isLong = 0;
                keybuffer->isLong = 0;
            } else if (db->database[i].isLong == 1) {
                isLong = 1;
                keybuffer->isLong = 1;
            } else{
                assert(0);
            }
            write(db->fd, keybuffer, KEYSIZE);
            free(keybuffer);
            keybuffer = NULL;         
            lseek(db->fd, JOURNALSIZE+KEYAREASIZE+clusindex*4*KB, SEEK_SET);
            if (isLong == 1) {
                write(db->fd, value, valuelength);
                if (16*MB-valuelength-1 > 0) {
                    lseek(db->fd, 16*MB-valuelength-1, SEEK_CUR);
                    write(db->fd, "0", 1);
                }
            }
            else {
                write(db->fd, value, valuelength);
                if (4*KB-valuelength-1 > 0) {
                    lseek(db->fd, 4*KB-valuelength-1, SEEK_CUR);
                    write(db->fd, "0", 1);
                }
            }
        }
    }
    fsync(db->fd);
    unload_database(db);
    return 0;
}

char *kvdb_get(struct kvdb *db, const char *key) {


    load_database(db);
    int clusNum = -1;
    int valueSize = -1;
    char* returned = NULL;
    for (int i = 0; db->database[i].valid != 0; i++) {
        int len = strtol(db->database[i].keysize, NULL, 16);
        char* keystored = malloc(len+1);
        keystored[len] = '\0';
        memcpy(keystored, db->database[i].KEY, len);
        if (strcmp(keystored, key) == 0) {
            clusNum = strtol(db->database[i].clusnum, NULL, 16);
            valueSize = strtol(db->database[i].valuesize, NULL, 16);
            break;
        }
    }

    if (clusNum >= 0 && valueSize >= 0) {
        returned = malloc(valueSize+1);
        returned[valueSize] = '\0';
        // lseek(db->fd, JOURNALSIZE+KEYAREASIZE+clusNum*4*KB,SEEK_SET);
        // read(db->fd, returned, valueSize);
        lseek(db->fd, JOURNALSIZE+KEYAREASIZE+clusNum*4*KB, SEEK_SET);
        read(db->fd, returned, valueSize);
    }
    unload_database(db);
    return returned;
}

int main() {

    struct kvdb *db = NULL;
    int instruction = -1;
    // int ttt = 0;
    char* longtext = malloc(8192);
    char* longtext2 = malloc(16384);
    char* longtext3 = malloc(4096);
    memset(longtext, 'b', 8192);
    memset(longtext2, 't', 16384);
    memset(longtext3, 'r', 4096);
    longtext[8192] = '\0';
    longtext2[16384] = '\0';
    longtext3[4096] = '\0';
    srand(time(NULL));
    char* keys[] = {"abc", "abcd","a", "b", "bd", "bde", "qwcvgsrgtre", "qwrvffzcbvce", "yrtbvfcthtxbvvcb"};
    char* values[] = {longtext,longtext2, "abc", "dfs", "werd", "scvxdf", longtext3, "qwcvgsrgtre", "qwrvffzcbvce", "yrtbvfcthtxbvvcb"};
    db = kvdb_open("lpr");
    int maps[9];
    int itre = 0;
    // kvdb_put(db, "yrtbvfcthtxbvvcb", "abc");
    // kvdb_put(db, "yrtbvfcthtxbvvcb", "werd");
    // kvdb_put(db, "qwcvgsrgtre", "dfs");

    // kvdb_put(db, "qwrvffzcbvce", "qwrvffzcbvce");
    // kvdb_put(db, "qwcvgsrgtre", longtext);
    // kvdb_put(db, "b", "abc");
    // kvdb_put(db, "qwcvgsrgtre", longtext);
    // kvdb_put(db, "yrtbvfcthtxbvvcb", "abc");
    // kvdb_put(db, "bde", "abc");
    // char* lpr = kvdb_get(db, "b");
    // assert(strcmp(lpr, "abc") == 0);
    // return 0;
    for (int i = 0; i < 9; i++)
        maps[i] = -1;
    while (true) {
        itre++;
        if (itre > 20000) {
            printf("PASS\n");
            return 0;
        } else {
            if (itre % 2000 == 0) {
                printf("%d0%% has complished\n", itre/2000);
            }
        }
        int instru = rand()%2;
        if (instru == 0) {
            int keyindex = rand() % 9;
            int valueindex = rand() % 10;
            kvdb_put(db, keys[keyindex], values[valueindex]);
            printf("put key:%s\tvalue:%s\n", keys[keyindex], values[valueindex]);

            maps[keyindex] = valueindex;
        } else {
            int keyindex = rand() % 9;
            char* returned = kvdb_get(db, keys[keyindex]);
            if (maps[keyindex] != -1) {
                
                printf("key:%s\tgetvalue:%s\tshoulebe:%s\n", keys[keyindex], returned, values[maps[keyindex]]);
            }

            if (maps[keyindex] == -1) {
                // assert(returned == NULL);

                assert(maps[keyindex] = -1); 
            } else {
                char* shouldbe = values[maps[keyindex]];
                fflush(stdout);
                assert((strcmp(returned, shouldbe) == 0));
                if (returned != NULL)
                    free(returned);
                returned = NULL;
            }
        }
    }
    kvdb_close(db);
    free(longtext);
    free(longtext2);
    free(longtext3);
    return 0;

    return 0;
}