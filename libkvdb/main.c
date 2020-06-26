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
// #include "kvdb.h"


// int main() {

//     struct kvdb *db = NULL;
//     int instruction = -1;
//     // int ttt = 0;
//     char* longtext = malloc(8192);
//     char* longtext2 = malloc(16384);
//     memset(longtext, 'b', 8192);
//     memset(longtext2, 't', 16384);
//     longtext[8192] = '\0';
//     longtext2[16384] = '\0';
//     srand(time(NULL));
//     char* keys[] = {"abc", "dfs", "werd", "scvxdf", "dfwreh", "qwcvgsrgtre", "qwrvffzcbvce", "yrtbvfcthtxbvvcb"};
//     char* values[] = {longtext,longtext2, "abc", "dfs", "werd", "scvxdf", "dfwreh", "qwcvgsrgtre", "qwrvffzcbvce", "yrtbvfcthtxbvvcb"};
//     db = kvdb_open("lpr");
//     int maps[8];
//     int itre = 0;
//     for (int i = 0; i < 8; i++)
//         maps[i] = -1;
//     while (true) {
//         itre++;
//         if (itre > 200) {
//             printf("PASS\n");
//             return 0;
//         } else {
//             if (itre % 20 == 0) {
//                 printf("%d0%% has complished\n", itre/20);
//             }
//         }
//         int instru = rand()%2;
//         if (instru == 0) {
//             int keyindex = rand() % 8;
//             int valueindex = rand() % 10;
//             kvdb_put(db, keys[keyindex], values[valueindex]);
//             // printf("put key:%s\tvalue:%s\n",keys[keyindex], values[valueindex]);
//             maps[keyindex] = valueindex;
//         } else {
//             int keyindex = rand() % 8;
//             char* returned = kvdb_get(db, keys[keyindex]);
//             // printf("get key:%s\n",keys[keyindex]);
//             if (returned == NULL || maps[keyindex] == -1) {
//                 if (returned!= NULL) {
//                     printf("%s\n", kvdb_get(db, keys[keyindex]));
//                 } 
//                 assert(maps[keyindex] = -1); 
//             } else {
//                 char* shouldbe = values[maps[keyindex]];
//                 if (strcmp(returned, shouldbe) != 0) {
//                     printf("len of the queried is%d\n", strlen(returned));
//                     printf("len of the real is%d\n", strlen(values[maps[keyindex]]));
//                     printf("queried value is %s\n", returned);
//                     printf("real value should be %s\n", values[maps[keyindex]]);
//                 }
//                 fflush(stdout);
//                 assert((strcmp(returned, shouldbe) == 0));
//                 if (returned != NULL)
//                     free(returned);
//                 returned = NULL;
//             }
//         }
//     }
//     kvdb_close(db);
//     free(longtext);
//     return 0;
//     // system("rm -f *.db");
//     return 0;
// }