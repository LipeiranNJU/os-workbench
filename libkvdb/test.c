#include <stdlib.h>
#include <stdio.h>
#include <string.h>
int main() {
    char* a = malloc(8192);
    memset(a, 'b', 8192);
    FILE* pfile = fopen("./bigvalue2.txt", "w+");
    fwrite(a, 1, 8192, pfile);
    fclose(pfile);
    return 0;
}