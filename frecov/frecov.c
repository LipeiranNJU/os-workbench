#include <stdio.h>
#include <assert.h>
#include <stdint.h>
#include <stdlib.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <sys/mman.h>
#include <string.h>
#include <unistd.h>
#include <sys/stat.h>
#include <stdbool.h>
#include <ctype.h>
#include <math.h>
#define B 1
//#define __DEBUG__
//#define __DDEBUG__
#define KB (1024 * B)
#define MB (1024 * KB)
#define GB (1024 * KB)
#ifdef __DEBUG__
#define print(...) printf(__VA_ARGS__)
#else
#define print(...) 
#endif

#ifdef __DDEBUG__
#define printk(...) printf(__VA_ARGS__)
#else
#define printk(...) 
#endif
#define beNotKown 0
#define isBMPDir 1
#define beNotUsed 2
#define beBMPContent 3

int BPB_BytsPerSec;
int BPB_SecPerClus;
int BPB_RootClus;
int BPB_FATSz32;
int BPB_HiddSec;
int BPB_RsvdSecCnt;
int BPB_NumFATs;
int offset;
int clusSize;
inline bool inFile(void* nowAddr, void* fileStart, int fileSize) {
    return ((intptr_t)(nowAddr) - (intptr_t)(fileStart)) < fileSize ? true : false;
}
inline int getClusterIndex(void* addr, void* start, int clusterSize) {
    return ((intptr_t)addr - (intptr_t) start) / clusterSize;
}
inline void* nextClus(void* Clus) {
    return (void*)((intptr_t)(Clus) + BPB_BytsPerSec * BPB_SecPerClus);
}

struct fat_header {
    uint8_t  BS_jmpBoot[3];
    uint8_t  BS_OEMName[8];
    uint16_t BPB_BytsPerSec;
    uint8_t BPB_SecPerClus;
    uint16_t BPB_RsvdSecCnt;
    uint8_t BPB_NumFATs;
    uint16_t BPB_RootEntCnt;
    uint16_t BPB_TotSec16;
    uint8_t BPB_Media;
    uint16_t BPB_FATSz16;
    uint16_t BPB_SecPerTrk;
    uint16_t BPB_NumHeads;
    uint32_t BPB_HiddSec;
    uint32_t BPB_TotSec32;
    uint32_t BPB_FATSz32;
    uint16_t BPB_ExtFlags;
    uint16_t BPB_FSVer;
    uint32_t BPB_RootClus;
    uint16_t BPB_FSInfo;
    uint16_t BPB_BkBootSec;
    uint8_t BPB_Reserved[12];
    uint8_t BS_DrvNum;
    uint8_t BS_Reserved;
    uint8_t BS_BootSig;
    uint32_t BS_VolID; 
    uint8_t BS_VolLab[11];
    uint8_t BS_FilSysType0;
    uint8_t BS_FilSysType1;
    uint8_t BS_FilSysType2;
    uint8_t BS_FilSysType3;
    uint8_t BS_FilSysType4;
    uint8_t BS_FilSysType5;
    uint8_t BS_FilSysType6;
    uint8_t BS_FilSysType7;
    uint8_t  padding[420];
    uint16_t Signature_word;
}__attribute__((packed));

struct BMPHeader {
    uint16_t bfType;
    uint32_t bfSize;
    uint16_t bfReserved1;
    uint16_t bfReserved2;
    uint32_t bfOffBits;
}__attribute__((packed));

struct BMPInfoHeader {
    uint32_t biSize;
    int32_t biWidth;
    uint32_t biHeight;
    uint16_t biPlanes;
    uint16_t biBitCount;
    uint32_t biCompression;
    uint32_t biSizeImage;
    int32_t biXPelsPerMeter;
    int32_t biYPelsPerMeter;
    uint32_t biClrUsed;
    uint32_t biClrImportant;
}__attribute__((packed));

struct FATShortDirectory {
    uint8_t DIR_Name[11];
    uint8_t DIR_Attr;
    uint8_t DIR_NTRes;
    uint8_t DIR_CrtTimeTenth;
    uint16_t DIR_CrtTime;
    uint16_t DIR_CrtDate;
    uint16_t DIR_LstAccDate;
    uint16_t DIR_FstClusHI;
    uint16_t DIR_WrtTime;
    uint16_t DIR_WrtDate;
    uint16_t DIR_FstClusLO;
    uint32_t DIR_FileSize;
}__attribute__((packed));

struct FATLongDirectory {
    uint8_t LDIR_Ord;
    uint16_t LDIR_Name1[5];
    uint8_t LDIR_Attr;
    uint8_t LDIR_Type;
    uint8_t LDIR_Chksum;
    uint16_t LDIR_Name2[6];
    uint16_t LDIR_FstClusLO;
    uint16_t LDIR_Name3[2];
}__attribute__((packed));

bool isFATShortDirectory(struct FATShortDirectory*);
char* readCompleteInfoFromFATShortDirectory(struct FATShortDirectory* pFATsd);
static inline struct FATLongDirectory* nextLongDirectory(struct FATLongDirectory* longDirectory){
    return (struct FATLongDirectory*)((intptr_t)(longDirectory) + sizeof(struct FATLongDirectory));
}
static inline struct FATShortDirectory* nextShortDirectory(struct FATShortDirectory* shortDirectory){
    return (struct FATShortDirectory*)((intptr_t)(shortDirectory) + sizeof(struct FATShortDirectory));
}
bool isValidFileName(char* name);
int dirClus[100];

void* getClusterFromIndex(int index_from_zero, void* initialClusterAddr) {
    return initialClusterAddr+index_from_zero*clusSize;
}
void initAttr(struct fat_header* pfatheader) {
    BPB_BytsPerSec = pfatheader->BPB_BytsPerSec;
    BPB_SecPerClus = pfatheader->BPB_SecPerClus;
    BPB_RootClus = pfatheader->BPB_RootClus;
    BPB_FATSz32 = pfatheader->BPB_FATSz32;
    BPB_HiddSec =pfatheader->BPB_HiddSec;
    BPB_RsvdSecCnt = pfatheader->BPB_RsvdSecCnt;
    BPB_NumFATs = pfatheader->BPB_NumFATs;
    clusSize = BPB_SecPerClus * BPB_BytsPerSec;
}
void dirClusAdd(int index) {
    assert(index >= 0);
    for (int i = 0; i < 100; i++) {
        if (dirClus[i] == -1) {
            dirClus[i] = index;
            return;
        }
    }
    assert(0);
}
int main(int argc, char *argv[]) {
    for (int i = 0; i < 100; i++) {
        dirClus[i] = -1;
    }
    char* fileName = argv[1];
    struct stat statbuf;
    stat(fileName,&statbuf);
    int size = statbuf.st_size;
    int fd = open(fileName, O_RDONLY, 0);
    struct fat_header* pfatheader =(struct fat_header*) mmap(NULL, size, PROT_READ, MAP_SHARED , fd, 0);
    initAttr(pfatheader);
    int totalClus = (size-offset)/(BPB_SecPerClus*BPB_BytsPerSec);
    int* clusStatus = malloc(totalClus);
    memset(clusStatus, 0, totalClus);
    offset = (BPB_RsvdSecCnt + BPB_NumFATs * BPB_FATSz32 + (BPB_RootClus - 2) * BPB_SecPerClus + BPB_HiddSec) * BPB_BytsPerSec;
    struct FATShortDirectory* pFATdir = (struct FATShortDirectory*)((intptr_t)pfatheader+offset);
    void* fatContentStart = (void*)((intptr_t)pfatheader+offset);
    bool skip = false;
    for (; (intptr_t)(pFATdir) < (intptr_t)(pfatheader)+size;pFATdir++) {
        assert((intptr_t)pFATdir-(intptr_t)pfatheader < pfatheader->BPB_TotSec32*pfatheader->BPB_BytsPerSec);
        if (isFATShortDirectory(pFATdir) == true) {
            char* magicNum = (char *) (offset + (uintptr_t)(pfatheader) + (pFATdir->DIR_FstClusLO - BPB_RootClus) * BPB_SecPerClus * BPB_BytsPerSec);
            struct BMPHeader* header = (struct BMPHeader*) magicNum;
            struct BMPInfoHeader* pBMInfoHeader = (struct BMPInfoHeader*) (header + 1);
            struct FATLongDirectory* pFATld = (struct FATLongDirectory*)(pFATdir - 1);
            char* picName = readCompleteInfoFromFATShortDirectory(pFATdir);
            char* prefix = "/tmp/";
            int size = strlen(prefix) + strlen(picName);
            char* abspath = malloc(sizeof(char) * (size + 1));
            memset(abspath, '\0', size + 1);
            strcat(strcat(abspath, prefix), picName);
            skip = false;
            for (int i = 0; i < strlen(abspath); i++) {
                if (!isalnum(abspath[i]) && abspath[i] != '.' && abspath[i] != '/') {
                    skip = true;
                    break;
                }
            }
            printf("%lx\n",(long)(((intptr_t)(pFATdir)-(intptr_t)fatContentStart)/clusSize));
            if (skip) {
                continue;
            }
            FILE* pfdpic = fopen(abspath, "w+");
            fwrite((void*) magicNum, 1, sizeof(*header), pfdpic);
            fclose(pfdpic);
            pfdpic = fopen(abspath, "a");
            fwrite((void*) pBMInfoHeader, 1, pBMInfoHeader->biSize, pfdpic);
            fclose(pfdpic);
            int lineWidthSize = pBMInfoHeader->biWidth*4;
            pfdpic = fopen(abspath, "a");
            // 位图数据区写入
            int picDataSize = header->bfSize - header->bfOffBits;
            void* picData = malloc(picDataSize);
            uint8_t* preLine = malloc(picDataSize);
            uint8_t* nowLine = malloc(picDataSize);
            uint8_t* laterLine = malloc(picDataSize);
            void* picDataStart = (void*) ((uintptr_t)(header) + header->bfOffBits);
            fwrite(picDataStart, 1, picDataSize/*(i+1)*lineWidthSize*/, pfdpic);
            fclose(pfdpic);
            char buf[41] = {};
            buf[40] = 0;
            char cmd[100] = {};
            int pipefds[2];
            if(pipe(pipefds) < 0){
		        perror("pipe");
                assert(0);
	        }
            int pid = fork();
            char* argv[3];
            argv[0] = "sha1sum",
            argv[1] = abspath;
            argv[2] = NULL;
            if (pid == 0) {
                close(pipefds[0]);
                dup2(pipefds[1], fileno(stderr));
                dup2(pipefds[1], fileno(stdout));
                execvp("sha1sum", argv);
            } else {
                close(pipefds[1]);
                read(pipefds[0], buf, 40);
                printf("%s    %s\n", buf, picName);
            }

        }
        fflush(stdout);
    }
    close(fd);
    return 0;    
}



bool isLegalInShort(char c) {
    if (c == 0x22 || c==0x2A || c==0x2B||c==0x2C ||c== 0x2E||c==0x2F ||c== 0x3A||c== 0x3B||c==0x3C ||c== 0x3D||c==0x3E ||c ==0x3F||c==0x5B||c==0x5C||c==0x5D||c==0x7C)
        return false;
    else
        return true;
    
}

bool isFATShortDirectory(struct FATShortDirectory* pFATdir) {
        if (strncmp((char *)&pFATdir->DIR_Name[8], "BMP", 3) == 0 && isalnum(pFATdir->DIR_Name[0]))
            if (pFATdir->DIR_NTRes == 0)
                if ((pFATdir->DIR_Attr >> 6) == 0)
                    if (pFATdir->DIR_FstClusHI == 0)
                        if (pFATdir->DIR_Attr != 0) {
                            for (int i = 1; i < 11; i++) {
                                if (pFATdir->DIR_Name[i] < 0x20 || pFATdir->DIR_Name[i]==0x22 || pFATdir->DIR_Name[i]==0x2A || pFATdir->DIR_Name[i]==0x2B || pFATdir->DIR_Name[i]==0x2C || pFATdir->DIR_Name[i]==0x2E || pFATdir->DIR_Name[i]==0x2F || pFATdir->DIR_Name[i]==0x3A || pFATdir->DIR_Name[i]==0x3B || pFATdir->DIR_Name[i]==0x3C || pFATdir->DIR_Name[i]==0x3D|| pFATdir->DIR_Name[i]==0x3E|| pFATdir->DIR_Name[i]==0x3F||pFATdir->DIR_Name[i]==0x5B|| pFATdir->DIR_Name[i]== 0x5C || pFATdir->DIR_Name[i]==0x5D || pFATdir->DIR_Name[i]== 0x7C)
                                    return false;
                            }

                            return true;
                        }

        return false;
}


char* readCompleteInfoFromFATShortDirectory(struct FATShortDirectory* pFATsd) {
    struct FATLongDirectory* pFATld =(struct FATLongDirectory*)(pFATsd-1);
    char c[140];
    int i = 0;
    memset(c, '\0', 140);
    c[i * 13 + 0] = (char) pFATld->LDIR_Name1[0];
    c[i * 13 + 1] = (char) pFATld->LDIR_Name1[1];
    c[i * 13 + 2] = (char) pFATld->LDIR_Name1[2];
    c[i * 13 + 3] = (char) pFATld->LDIR_Name1[3];
    c[i * 13 + 4] = (char) pFATld->LDIR_Name1[4];
    c[i * 13 + 5] = (char) pFATld->LDIR_Name2[0];
    c[i * 13 + 6] = (char) pFATld->LDIR_Name2[1];
    c[i * 13 + 7] = (char) pFATld->LDIR_Name2[2];
    c[i * 13 + 8] = (char) pFATld->LDIR_Name2[3];
    c[i * 13 + 9] = (char) pFATld->LDIR_Name2[4];
    c[i * 13 + 10] = (char) pFATld->LDIR_Name2[5];
    c[i * 13 + 11] = (char) pFATld->LDIR_Name3[0];
    c[i * 13 + 12] = (char) pFATld->LDIR_Name3[1];
    if (pFATld->LDIR_Ord > 0x40){
        int size = strlen(c);
        char * t = malloc(sizeof(char)*140);
        memset(t, '\0', 140);
        memcpy(t, c, size);
        return t;
    }
    else {
        pFATld = pFATld - 1;
        int i = 1;
        c[i * 13 + 0] = (char) pFATld->LDIR_Name1[0];
        c[i * 13 + 1] = (char) pFATld->LDIR_Name1[1];
        c[i * 13 + 2] = (char) pFATld->LDIR_Name1[2];
        c[i * 13 + 3] = (char) pFATld->LDIR_Name1[3];
        c[i * 13 + 4] = (char) pFATld->LDIR_Name1[4];
        c[i * 13 + 5] = (char) pFATld->LDIR_Name2[0];
        c[i * 13 + 6] = (char) pFATld->LDIR_Name2[1];
        c[i * 13 + 7] = (char) pFATld->LDIR_Name2[2];
        c[i * 13 + 8] = (char) pFATld->LDIR_Name2[3];
        c[i * 13 + 9] = (char) pFATld->LDIR_Name2[4];
        c[i * 13 + 10] = (char) pFATld->LDIR_Name2[5];
        c[i * 13 + 11] = (char) pFATld->LDIR_Name3[0];
        c[i * 13 + 12] = (char) pFATld->LDIR_Name3[1];
            if (pFATld->LDIR_Ord > 0x40) {
                int size = strlen(c);
                char * t = malloc(sizeof(char)*140);
                memset(t, '\0', 140);
                t[size] = 0;
                memcpy(t, c, size);
                return t;
            } else {
                pFATld = pFATld - 1;
                int i = 2;
                c[i * 13 + 0] = (char) pFATld->LDIR_Name1[0];
                c[i * 13 + 1] = (char) pFATld->LDIR_Name1[1];
                c[i * 13 + 2] = (char) pFATld->LDIR_Name1[2];
                c[i * 13 + 3] = (char) pFATld->LDIR_Name1[3];
                c[i * 13 + 4] = (char) pFATld->LDIR_Name1[4];
                c[i * 13 + 5] = (char) pFATld->LDIR_Name2[0];
                c[i * 13 + 6] = (char) pFATld->LDIR_Name2[1];
                c[i * 13 + 7] = (char) pFATld->LDIR_Name2[2];
                c[i * 13 + 8] = (char) pFATld->LDIR_Name2[3];
                c[i * 13 + 9] = (char) pFATld->LDIR_Name2[4];
                c[i * 13 + 10] = (char) pFATld->LDIR_Name2[5];
                c[i * 13 + 11] = (char) pFATld->LDIR_Name3[0];
                c[i * 13 + 12] = (char) pFATld->LDIR_Name3[1];
                if (pFATld->LDIR_Ord > 0x40) {
                    int size = strlen(c);
                    char * t = malloc(sizeof(char)*140);
                    memset(t, '\0', 140);
                    memcpy(t, c, size);
                    return t;
                }
            }
    }
    return NULL;
}
int cmpfunc (const void * a, const void * b) {
   return ( *(double*)a - *(double*)b );
}
//void lineCmp(uint8_t* preLine, uint8_t* nowLine,uint8_t* latterLine, int size) {
//    int width = size/4;
//    int sum[3] = {0};
//    int Gx_B = 0;
//    int Gy_B = 0;
//    int G_B = 0;
//    int Gx_G = 0;
//    int Gy_G = 0;
//    int G_G = 0;
//    int Gx_R = 0;
//    int Gy_R = 0;
//    int G_R = 0;
//    double* list = malloc(sizeof(double)*(width-1));
//    double linesum = 0;
//    for (int i = 1; i < width-1; i++){
//        Gx_B = 2*nowLine[(i+1)*4+0] - 2*nowLine[(i-1)*4+0] + preLine[(i+1)*4+0] - preLine[(i-1)*4+0] + latterLine[(i+1)*4+0] - latterLine[(i-1)*4+0];
//        Gy_B = 2*latterLine[(i)*4+0] - 2*preLine[(i)*4+0] - preLine[(i+1)*4+0] - preLine[(i-1)*4+0] + latterLine[(i+1)*4+0] + latterLine[(i-1)*4+0];
//        G_B = sqrt(1.0*Gx_B*Gx_B+1.0*Gy_B*Gy_B);
//        sum[0]+=G_B;
//
//
//        Gx_G = 2*nowLine[(i+1)*4+1] - 2*nowLine[(i-1)*4+1] + preLine[(i+1)*4+1] - preLine[(i-1)*4+1] + latterLine[(i+1)*4+1] - latterLine[(i-1)*4+1];
//        Gy_G = 2*latterLine[(i)*4+1] - 2*preLine[(i)*4+1] - preLine[(i+1)*4+1] - preLine[(i-1)*4+1] + latterLine[(i+1)*4+1] + latterLine[(i-1)*4+1];
//        G_G = sqrt(1.0*Gx_G*Gx_G+1.0*Gy_G*Gy_G);
//        sum[1]+=G_G;
//
//
//        Gx_R = 2*nowLine[(i+1)*4+2] - 2*nowLine[(i-1)*4+2] + preLine[(i+1)*4+2] - preLine[(i-1)*4+2] + latterLine[(i+1)*4+2] - latterLine[(i-1)*4+2];
//        Gy_R = 2*latterLine[(i)*4+2] - 2*preLine[(i)*4+2] - preLine[(i+1)*4+2] - preLine[(i-1)*4+2] + latterLine[(i+1)*4+2] + latterLine[(i-1)*4+2];
//        G_R = sqrt(1.0*Gx_R*Gx_R+1.0*Gy_R*Gy_R);
//        sum[2]+=G_R;
//        list[i-1] = sqrt(1.0*sum[0]*sum[0]+1.0*sum[1]*sum[1]+1.0*sum[2]*sum[2]);
//        double length = sqrt(1.0*sum[0]*sum[0]+1.0*sum[1]*sum[1]+1.0*sum[2]*sum[2]);
//        linesum += length;
//    }
//    qsort(list, width-1,sizeof(double),cmpfunc);
//    printf("\n");
//    for (int i = 0; i < width-1;i++)
//        printf("%lf ",list[i]);
//
//    printf("\n%lf\n", linesum/(width-2));
//}
bool isValidFileName(char* name) {
    for (int i = 0; i < strlen(name); i++) {
        if (!isalnum(name[i]) && name[i] != '.' && name[i] != '/') {
            return false;
        }
    }
    if (name[strlen(name)-1] == 'p' && name[strlen(name)-2] == 'm' &&name[strlen(name)-3] == 'b'&&name[strlen(name)-4] == '.')
        return true;
    else
        return false;
    
}