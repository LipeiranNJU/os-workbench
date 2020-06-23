// #include <stdio.h>
// #include <assert.h>
// #include <stdio.h>
// #include <string.h>
// #include <unistd.h>
// #include <stdlib.h>
// #include <fcntl.h>
// #include <sys/stat.h>

// char* keyIdentifier = "@lprylkey";
// char* valueIdentifier = "^lprylvalue";
// char* endIdentifier = "$lprylend";

// // struct data {
// //   void* dataSec;
// //   struct data* next;
// //   void* key;
// // };
// struct record {
//   char* key;
//   char* value;
//   struct record *next;
// };
// // struct dataList {
// //   struct data* dataHead;
// //   struct dataList* next;
// // };
// struct kvdb {
//   // your definition here
//   int fd;
//   struct record *recordListHead;
// };

// struct kvdb *kvdb_open(const char *filename) {
//     char workpath[1000];
//     strcpy(workpath, "/home/lpr/os-workbench/libkvdb/");
//     int fd = open(strcat(strcat(workpath, filename), ".db"), O_RDWR | O_CREAT, 0777);
//     long offset = 0;
//     struct stat statbuf;  
//     stat(workpath, &statbuf);  
//     int fileSize = statbuf.st_size;  
//     printf("filesize:%d\n", fileSize);

//     printf("Now is opening database:%s\n", workpath);
//     struct kvdb* pkvdb = malloc(sizeof(struct kvdb));
//     pkvdb->recordListHead = malloc(sizeof(struct record));
//     pkvdb->recordListHead->next = NULL;
//     pkvdb->recordListHead->key = malloc(sizeof(char));
//     pkvdb->recordListHead->value = malloc(sizeof(char));
//     pkvdb->recordListHead->key = '\0';
//     pkvdb->recordListHead->value = '\0';
//     // assert(0);
//     pkvdb->fd = fd;
//     system("pwd");
//     lseek(fd, 0, SEEK_SET);
//     if (fileSize > 9) {
//         struct record* tmptail;
//         for (tmptail = pkvdb->recordListHead; tmptail->next != NULL; tmptail = tmptail->next) ;
//         while (offset < fileSize) {
//             struct record* tmprecord = malloc(sizeof(struct record));
//             tmprecord->next = NULL;
//             tmptail->next = tmprecord;
//             tmptail = tmptail->next;
//             char tmpIdentifier[50];
//             char sizeString[9];
//             char **tmp = NULL;
//             memset(sizeString, '\0', 9);
//             memset(tmpIdentifier, '\0', 50);
//             read(pkvdb->fd, tmpIdentifier, 9);
//             offset += 9;
//             assert(strcmp(tmpIdentifier, keyIdentifier) == 0);
//             read(pkvdb->fd, sizeString, 8);
//             offset += 8;
//             long keySize = strtol(sizeString, tmp, 16);
//             printf("keySize:%lx\n", keySize);
//             tmprecord->key = malloc(sizeof(keySize)+1);
//             tmprecord->key[keySize] = '\0';
//             read(pkvdb->fd, tmprecord->key, keySize);
//             offset += keySize;
//             memset(tmpIdentifier, '\0', 50);
//             read(pkvdb->fd, tmpIdentifier, 11);
//             offset += 11;
//             assert(strcmp(tmpIdentifier, valueIdentifier) == 0);
//             read(pkvdb->fd, sizeString, 8);
//             offset += 8;
//             long valueSize = strtol(sizeString, tmp, 16);
//             printf("valueSize:%lx\n", valueSize);
//             tmprecord->value = malloc(sizeof(valueSize)+1);
//             tmprecord->value[valueSize] = '\0';
//             read(pkvdb->fd, tmprecord->value, valueSize);
//             offset += valueSize;
//             memset(tmpIdentifier, '\0', 50);
//             read(pkvdb->fd, tmpIdentifier, 9);
//             offset += 9;
//             assert(strcmp(tmpIdentifier, endIdentifier) == 0);
//         }

//     } else {
//         assert(0);
//     }
    
//     // assert(0);
//     lseek(fd, 0, SEEK_END);
//     // assert(0);
//     return pkvdb;
// }

// int kvdb_close(struct kvdb *db) {
//     return close(db->fd);
// }

// int kvdb_put(struct kvdb *db, const char *key, const char *value) {
//     int keysize = strlen(key);
//     int valuesize = strlen(value);
//     char keysizestring[9];
//     char valuesizestring[9];
//     memset(keysizestring, '\0', 9);
//     memset(valuesizestring, '\0', 9);
//     sprintf(keysizestring, "%08x", keysize);
//     sprintf(valuesizestring, "%08x", valuesize);

//     write(db->fd, keyIdentifier, strlen(keyIdentifier));
//     write(db->fd, keysizestring, 8);
//     write(db->fd, key, strlen(key));
//     write(db->fd, valueIdentifier, strlen(valueIdentifier));
//     write(db->fd, valuesizestring, 8);
//     write(db->fd, value, strlen(value));
//     write(db->fd, endIdentifier, strlen(endIdentifier));
//     fsync(db->fd);
//     return 0;
// }

// char *kvdb_get(struct kvdb *db, const char *key) {
//     struct record *tmp;
//     for (tmp = db->recordListHead; tmp != NULL; tmp = tmp->next) {
//         if (tmp->key == NULL) 
//             continue;
//         if (strcmp(key, tmp->key) == 0) 
//             return tmp->value;
//     }
//     return NULL;
// }

// enum {QUIT, INSERT, DELETE, OPEN, CLOSE, QUERY};

// int main() {
//     struct kvdb *db = NULL;
//     int instruction = -1;
//     while (instruction != QUIT) {
//         printf("Please tell me what you want:\n");
//         printf("[%d] QUIT\n", QUIT);
//         printf("[%d] INSERT\n", INSERT);
//         printf("[%d] DELETE\n", DELETE);
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
//         } else if (instruction == INSERT) {
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