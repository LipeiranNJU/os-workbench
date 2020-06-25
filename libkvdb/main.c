// #include <stdio.h>
// #include <assert.h>
// #include <stdio.h>
// #include <string.h>
// #include <unistd.h>
// #include <stdlib.h>
// #include <fcntl.h>
// #include <sys/stat.h>
// #include <stdint.h>
// #include <stdbool.h>
// #include <time.h>
// #define B 1
// #define KB (1024 * B)
// #define MB (1024 * KB)
// #define JOURNALSIZE 10
// #define KEYSIZE (1 + 128 * B + 9 + 9 + 9 + 1)
// #define KEYITEMS (50)
// #define KEYAREASIZE (KEYSIZE * KEYITEMS)


// enum {UPDATE, INSERT};

// struct record {
//     char valid;
//     char KEY[128];
//     char keysize[9];
//     char valuesize[9];
//     char clusnum[9];
//     char isLong;
// };

// struct kvdb {
//   // your definition here
//   int fd;
//   struct record *database;
// };
// char workpath[1000];
// enum {U, M, H, S};

// void goto_journal(const struct kvdb *db) {
//     lseek(db->fd, 0, SEEK_SET);
// }

// void goto_keyarea(const struct kvdb *db) {
//     lseek(db->fd, JOURNALSIZE, SEEK_SET);
// }

// void goto_clusters(const struct kvdb *db) {
//     lseek(db->fd, (JOURNALSIZE+KEYAREASIZE), SEEK_SET);
// }

// void load_database(struct kvdb* db) {
//     db->database= malloc(sizeof(char)*KEYAREASIZE);
//     lseek(db->fd, JOURNALSIZE, SEEK_SET);
//     read(db->fd, db->database, KEYAREASIZE);
//     return ;
// }

// void setkeyondisk(struct kvdb* db,const char* keyname, const char* valuename, const int keyindex_i) {
//     int keySizeNum = strlen(keyname);
//     int valueSizeNum = strlen(valuename);
//     char keysize[9];
//     char valuesize[9];
//     sprintf(keysize, "%07x", keySizeNum);
//     sprintf(valuesize, "%07x", valueSizeNum);
//     int end = lseek(db->fd, 0, SEEK_END);
//     int clusnum = (end-JOURNALSIZE-KEYAREASIZE)/(4*KB);
//     printf("offset:%d\n",(end-JOURNALSIZE-KEYSIZE));
//     printf("clusnum:%d\n", clusnum);
//     lseek(db->fd, JOURNALSIZE+keyindex_i*KEYSIZE, SEEK_SET);
//     struct record* keybuffer = malloc(sizeof(char)*KEYSIZE);
//     sprintf(keybuffer->clusnum, "%07x", clusnum);
//     keybuffer->valid = '1';
//     memcpy(keybuffer->keysize, keysize, 9);
//     memcpy(keybuffer->valuesize, valuesize, 9);
//     memcpy(keybuffer->KEY, keyname, keySizeNum);
//     printf("keysize:%s\n", keysize);
//     if (valueSizeNum > 4*KB) {
//         keybuffer->isLong = '1';
//     }
//     write(db->fd, keybuffer, KEYSIZE);
//     fsync(db->fd);
//     free(keybuffer);
//     keybuffer = NULL;
// }


// void unload_database(struct kvdb *db) {
//     free(db->database);
//     db->database = NULL;
//     return ;
// }
// struct kvdb *kvdb_open(const char *filename) {
//     strcpy(workpath, "/home/lpr/os-workbench/libkvdb/");
//     strcat(strcat(workpath, filename), ".db");
//     int fd = open(workpath, O_RDWR | O_CREAT, 0777);
//     // long offset = 0;
//     struct stat statbuf;  
//     stat(workpath, &statbuf);  
//     struct kvdb* pkvdb = malloc(sizeof(struct kvdb));
//     pkvdb->fd = fd;
//     int fileSize = statbuf.st_size;  
//     // printf("filesize:%d\n", fileSize);
//     if (fileSize == 0) {
//         char* zeros = malloc(sizeof(char)*(16*MB+1*KB));
//         memset(zeros, '0', JOURNALSIZE);
//         write(pkvdb->fd, zeros, JOURNALSIZE);
//         fsync(pkvdb->fd);
//         free(zeros);
//         zeros = NULL;

//         goto_keyarea(pkvdb);
//         zeros = malloc(sizeof(char)*KEYAREASIZE);
//         memset(zeros, '0', KEYAREASIZE);
//         write(pkvdb->fd, zeros, KEYAREASIZE);
//         fsync(pkvdb->fd);
//         free(zeros);
//         zeros = NULL;
//     }
//     // printf("Now is opening database:%s\n", workpath);
//     memset(workpath, '\0', 1000);
//     return pkvdb;
// }

// int kvdb_close(struct kvdb *db) {

//     close(db->fd);
//     free(db);
//     db = NULL;
//     return 0;
// }

// int kvdb_put(struct kvdb *db, const char *key, const char *value) {
//     load_database(db);
//     int i;
//     int valuelength = strlen(value);
//     for (i = 0; db->database[i].valid != '0'; i++) {
//         // printf("NAME:%s\n", db->database[i].KEY);
//         if (strcmp(db->database[i].KEY, key) == 0) {
//             break;
//         }
//     }
//     // printf("key:%s\ti:%d\n", key, i);

//     if (db->database[i].valid == '0') {
//         setkeyondisk(db, key, value, i);
//         lseek(db->fd, 0, SEEK_END);
//         if (valuelength <= 4*KB) {
//             char* valuebuffer = malloc(sizeof(char) * 4 * KB);
//             memcpy(valuebuffer, value, valuelength);
//             write(db->fd, valuebuffer, 4*KB);
//             fsync(db->fd);
//             free(valuebuffer);
//             valuebuffer = NULL;
//         } else {
//             char* valuebuffer = malloc(sizeof(char) * 16 * MB);
//             memcpy(valuebuffer, value, valuelength);
//             write(db->fd, valuebuffer, 16 * MB);
//             fsync(db->fd);
//             free(valuebuffer);
//             valuebuffer = NULL;
//         }
//     } else {
//         int rawsize = strtol(db->database[i].keysize, NULL, 16);
//         if (db->database[i].isLong != '1' && valuelength >=4*KB) {
//             setkeyondisk(db, key, value, i);
//             lseek(db->fd, 0, SEEK_END);
//             char* valuebuffer = malloc(sizeof(char) * 16 * MB);
//             memcpy(valuebuffer, value, valuelength);
//             write(db->fd, valuebuffer, 16 * MB);
//             fsync(db->fd);
//             free(valuebuffer);
//             valuebuffer = NULL;
//         } else {
//             int keySizeNum = strlen(key);
//             int valueSizeNum = strlen(value);
//             char keysize[9];
//             char valuesize[9];
//             sprintf(keysize, "%07x", keySizeNum);
//             sprintf(valuesize, "%07x", valueSizeNum);
//             lseek(db->fd, JOURNALSIZE+i*KEYSIZE, SEEK_SET);
//             struct record* keybuffer = malloc(sizeof(char)*KEYSIZE);
//             keybuffer->valid = '1';
//             memcpy(keybuffer->keysize, keysize, 9);
//             memcpy(keybuffer->valuesize, valuesize, 9);
//             memcpy(keybuffer->KEY, key, keySizeNum);
//             memcpy(keybuffer->clusnum, db->database[i].clusnum, 9);
//             // printf("keysize:%s\n", keysize);
//             write(db->fd, keybuffer, KEYSIZE);
//             fsync(db->fd);
//             free(keybuffer);
//             keybuffer = NULL;
//             int clusindex = strtol(db->database[i].clusnum, NULL, 16);
//             lseek(db->fd, JOURNALSIZE+KEYAREASIZE+clusindex*4*KB, SEEK_SET);
//             write(db->fd, value, valuelength);
//             fsync(db->fd);
//         }
//     }
//     unload_database(db);
//     return 0;
// }

// char *kvdb_get(struct kvdb *db, const char *key) {
//     load_database(db);
//     int clusNum = -1;
//     int valueSize = -1;
//     char* returned = NULL;
//     for (int i = 0; db->database[i].valid != '0'; i++) {
//         if (strncmp(db->database[i].KEY, key, strtol(db->database[i].keysize, NULL, 16)) == 0) {
//             clusNum = strtol(db->database[i].clusnum, NULL, 16);
//             valueSize = strtol(db->database[i].valuesize, NULL, 16);
//             break;
//         }
//     }
//     if (strcmp(key, "lpr") == 0) {
//         printf("CLUSNUM:%d\tVALUESIZE:%d\n", clusNum, valueSize);
//     }
//     if (clusNum >= 0 && valueSize >=0) {
//         returned = malloc(sizeof(char)*(valueSize+1));
//         returned[valueSize] = '\0';
//         lseek(db->fd, JOURNALSIZE+KEYAREASIZE+clusNum*4*KB,SEEK_SET);
//         read(db->fd, returned, valueSize);
//     }
//     unload_database(db);
//     return returned;
// }

// enum {QUIT, PUT, OPEN, CLOSE, QUERY};

// int main() {

//     struct kvdb *db = NULL;
//     int instruction = -1;
//     // int ttt = 0;
//     char* lpr = "lpr";
//     char* wzl = "wzl";
//     char* yl = "yl";
//     char* skyxmt = "skyxmt";
//     char* xmyhj = "xmyhj";
//     char* csjgsqzhx = "csjgsqzhx";
//     char* lnxe = "lnxe";
//     char* longtext = malloc(8193);
//     memset(longtext, 'b', 8192);
//     longtext[8192] = '\0';
//     srand(time(NULL));
//     char* keys[] = {"abc", "dfs", "werd", "scvxdf", "dfwreh", "qwcvgsrgtre", "qwrvffzcbvce", "yrtbvfcthtxbvvcb"};
//     char* values[] = {longtext, "abc", "dfs", "werd", "scvxdf", "dfwreh", "qwcvgsrgtre", "qwrvffzcbvce", "yrtbvfcthtxbvvcb"};
//     db = kvdb_open("lpr");
//     kvdb_put(db, lpr, skyxmt);
//     kvdb_put(db, lpr, yl);
//     kvdb_put(db, lnxe, lpr);
//     kvdb_put(db, lnxe, xmyhj);
//     kvdb_put(db, lnxe, yl);
//     kvdb_put(db, xmyhj, csjgsqzhx);
//     kvdb_put(db, csjgsqzhx, longtext);
//     kvdb_put(db, csjgsqzhx, wzl);
//     kvdb_put(db, csjgsqzhx, skyxmt);
//     // printf("test:value:%s\n", kvdb_get(db, lpr));
//     // printf("test:value:%s\n", kvdb_get(db, lnxe));
//     // printf("test:value:%s\n", kvdb_get(db, xmyhj));
//     // printf("test:value:%s\n", kvdb_get(db, csjgsqzhx));
//     int maps[8];
//     for (int i = 0; i < 8; i++)
//         maps[i] = -1;
//     while (true) {
//         int instru = rand()%2;
//         if (instru == 0) {
//             int keyindex = rand() % 8;
//             int valueindex = rand() % 9;
//             kvdb_put(db, keys[keyindex], values[valueindex]);
//             printf("put key:%s\tvalue%s\n",keys[keyindex], values[valueindex]);
//             maps[keyindex] = valueindex;
//         } else {
//             int keyindex = rand() % 8;
//             printf("get key:%s\t%s\n",keys[keyindex], values[maps[keyindex]]);
//             if (maps[keyindex] == -1) {
//                 if (kvdb_get(db, keys[keyindex]) != NULL) {
//                     printf("%s\n", kvdb_get(db, keys[keyindex]));
//                 } 
//                 assert(kvdb_get(db, keys[keyindex]) == NULL); 
//             } else {
//                 char* returned = kvdb_get(db, keys[keyindex]);
//                 if (strcmp(returned, values[maps[keyindex]]) != 0) {
//                     printf("real value should be %s\n", values[maps[keyindex]]);
//                 }
//                 assert(strcmp(returned, values[maps[keyindex]]) == 0);
//                 free(returned);
//                 returned = NULL;
//             }
//         }
//     }
//     kvdb_close(db);
//     free(longtext);
//     return 0;
//     while (instruction != QUIT) {
//         printf("Please tell me what you want:\n");
//         printf("[%d] QUIT\n", QUIT);
//         printf("[%d] PUT\n", PUT);
//         printf("[%d] OPEN\n", OPEN);
//         printf("[%d] CLOSE\n", CLOSE);
//         printf("[%d] QUERY\n", QUERY);
//         scanf("%d", &instruction);
//         if (instruction == OPEN) {
//             printf("Please ENTER NAME:\n");
//             char name[100];
//             scanf("%s", name);
//             printf("NAME:%s\n", name);
//             db = kvdb_open(name);
//             assert(db != NULL);
//         } else if (instruction == CLOSE) {
//             int closeStatus = kvdb_close(db);
//             assert(closeStatus == 0);
//             break;
//         } else if (instruction == PUT) {
//             char key[100];
//             char value[100];
//             printf("Please ENTER KEY:\n");
//             scanf("%s", key);
//             printf("Please ENTER value:\n");
//             scanf("%s", value);
//             kvdb_put(db, key, value);
//         } else if (instruction == QUERY) {
//             char key[100];
//             printf("Please ENTER KEY:\n");
//             scanf("%s", key);
//             char* v = kvdb_get(db, key);
//             printf("value of this key is %s\n", v);
//         }
        
//     }

//     // system("rm -f *.db");
//     return 0;
// }