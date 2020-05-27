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
#define Unknown (-1)
#define DirEntry 1
#define beNotUsed 2
#define BMPContent 3

int BPB_BytsPerSec;
int BPB_SecPerClus;
int BPB_RootClus;
int BPB_FATSz32;
int BPB_HiddSec;
int BPB_RsvdSecCnt;
int BPB_NumFATs;
int offset;
int clusSize;
inline bool inFile(const void* nowAddr, const void* fileStart, int fileSize) {
    return ((intptr_t)(nowAddr) - (intptr_t)(fileStart)) < fileSize ? true : false;
}
inline int getClusterIndex(const void* addr, const void* start, int clusterSize) {
    return ((intptr_t)addr - (intptr_t) start) / clusterSize;
}
inline void* nextClus(const void* Clus) {
    return (void*)((intptr_t)(Clus) + BPB_BytsPerSec * BPB_SecPerClus);
}

struct FATHeader {
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
bool isFATLongDirectory(const struct FATLongDirectory* pFATldir);
bool isFATShortDirectory(const struct FATShortDirectory*);
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
void initAttr(struct FATHeader* pfatheader) {
    BPB_BytsPerSec = pfatheader->BPB_BytsPerSec;
    BPB_SecPerClus = pfatheader->BPB_SecPerClus;
    BPB_RootClus = pfatheader->BPB_RootClus;
    BPB_FATSz32 = pfatheader->BPB_FATSz32;
    BPB_HiddSec = pfatheader->BPB_HiddSec;
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
int main (int argc, char* argv[]) {
    char* imgName = argv[1];
    struct stat statbuf;
    stat(imgName, &statbuf);
    int imgSize = statbuf.st_size;
    int imgFd = open(imgName, O_RDONLY, 0);
    struct FATHeader* pFATHeader = (struct FATHeader *) mmap(NULL, imgSize, PROT_READ, MAP_SHARED, imgFd, 0);

    BPB_BytsPerSec = 512;
    BPB_SecPerClus = 8;
    BPB_RootClus = pFATHeader->BPB_RootClus;
    BPB_FATSz32 = pFATHeader->BPB_FATSz32;
    BPB_HiddSec = pFATHeader->BPB_HiddSec;
    BPB_RsvdSecCnt = pFATHeader->BPB_RsvdSecCnt;
    BPB_NumFATs = pFATHeader->BPB_NumFATs;
    clusSize = BPB_BytsPerSec * BPB_SecPerClus;
    int imgOffset = (BPB_RsvdSecCnt+BPB_NumFATs*BPB_FATSz32+(BPB_RootClus-2)*BPB_SecPerClus+BPB_HiddSec)*BPB_BytsPerSec;
    struct FATShortDirectory* pFATshdir = (void* )pFATHeader + imgOffset;
    void* imgDataStart = (void* )pFATHeader + imgOffset;
    int imgDataSize = imgSize - imgOffset;
    int tmpi = 0;
    int clusNum = imgDataSize / clusSize;
    int* cluses = malloc(sizeof(int)*clusNum);
    for (int i = 0; i < clusNum; i++) 
        cluses[i] = Unknown;
    for (void* cluster = imgDataStart; inFile(cluster, imgDataStart, imgDataSize); cluster = nextClus(cluster)) {
        int countsh = 0;
        int countl = 0;
        for (struct FATShortDirectory* ptmp = cluster; inFile(ptmp, cluster, clusSize); ptmp++) {
            if (isFATShortDirectory(ptmp)) {
                char nameTmp[12];
                memcpy(nameTmp, ptmp->DIR_Name, 11);
                nameTmp[11] = '\0';
                tmpi++;
                countsh++;
                if (isFATLongDirectory((void*)(ptmp-1)))
                    countl++;
                if (isFATLongDirectory((void*)(ptmp-2)))
                    countl++;
            }
        }
        if (countsh > 6 && countl >= countsh) {
            int index = getClusterIndex(cluster, imgDataStart, clusSize);
            // printf("%p\n", cluster);
            cluses[index] = DirEntry;
        }
    }
    for (int i = 0; i < clusNum; i++) {
        if (cluses[i] == DirEntry) {
            // printf("index:%d\n", i);
            void* cluster = getClusterFromIndex(i, imgDataStart);
            // printf("%p\n", cluster);
            int j = 0;
            for (struct FATShortDirectory* ptmp = cluster; inFile(ptmp, cluster, clusSize); ptmp++) {
                if (isFATShortDirectory(ptmp) && isFATLongDirectory((void*)(ptmp-1))) {
                    char nameTmp[12];
                    memcpy(nameTmp, ptmp->DIR_Name, 11);
                    nameTmp[11] = '\0';
                    // printf("%s\t%d\n", nameTmp, ++j);
                    // char* name = readCompleteInfoFromFATShortDirectory(ptmp);
                    // if (name != NULL) {
                    //     for (int i = 0; i < 40; i++) 
                    //         printf("a");
                        
                    //     printf("   %s\n", name);
                    // }
                }
            }
        }
    }
    return 0;
}

bool isFATShortDirectory(const struct FATShortDirectory* ptmp) {
    if (strncmp((char*)&ptmp->DIR_Name[8], "BMP", 3) == 0) {
        return true;
    }
    return false;
}

bool isFATLongDirectory(const struct FATLongDirectory* pFATldir) {

    if (pFATldir->LDIR_FstClusLO != 0 || pFATldir->LDIR_Type != 0)
        return false;
    else    
        return true;
}
char* readCompleteInfoFromFATShortDirectory(struct FATShortDirectory* pFATsd) {
    
    char* name = malloc(sizeof(char) * 200);
    memset(name, '\0', 200);
    struct FATLongDirectory* pFATld = (struct FATLongDirectory*) (pFATsd-1);
    int i = -1;
    while((pFATld->LDIR_Ord >> 4)==0) {
        i += 1;
        name[i*13+0] = (char) pFATld->LDIR_Name1[0];
        name[i*13+1] = (char) pFATld->LDIR_Name1[1];
        name[i*13+2] = (char) pFATld->LDIR_Name1[2];
        name[i*13+3] = (char) pFATld->LDIR_Name1[3];
        name[i*13+4] = (char) pFATld->LDIR_Name1[4];
        name[i*13+5] = (char) pFATld->LDIR_Name2[0];
        name[i*13+6] = (char) pFATld->LDIR_Name2[1];
        name[i*13+7] = (char) pFATld->LDIR_Name2[2];
        name[i*13+8] = (char) pFATld->LDIR_Name2[3];
        name[i*13+9] = (char) pFATld->LDIR_Name2[4];
        name[i*13+10] = (char) pFATld->LDIR_Name2[5];
        name[i*13+11] = (char) pFATld->LDIR_Name3[0];
        name[i*13+12] = (char) pFATld->LDIR_Name3[1];
        pFATld = pFATld-1;
    }
    i += 1;
    name[i*13+0] = (char) pFATld->LDIR_Name1[0];
    name[i*13+1] = (char) pFATld->LDIR_Name1[1];
    name[i*13+2] = (char) pFATld->LDIR_Name1[2];
    name[i*13+3] = (char) pFATld->LDIR_Name1[3];
    name[i*13+4] = (char) pFATld->LDIR_Name1[4];
    name[i*13+5] = (char) pFATld->LDIR_Name2[0];
    name[i*13+6] = (char) pFATld->LDIR_Name2[1];
    name[i*13+7] = (char) pFATld->LDIR_Name2[2];
    name[i*13+8] = (char) pFATld->LDIR_Name2[3];
    name[i*13+9] = (char) pFATld->LDIR_Name2[4];
    name[i*13+10] = (char) pFATld->LDIR_Name2[5];
    name[i*13+11] = (char) pFATld->LDIR_Name3[0];
    name[i*13+12] = (char) pFATld->LDIR_Name3[1];
    for (int j = 12; j > -1; j--) {
        if (name[i*13+j] == 'p' && name[i*13+j-1] == 'm' && name[i*13+j-2] == 'b' && name[i*13+j] == '.' ) {
            name[i*13+j+1]='\0';
            break;
        }
    }
    for (int i = 0; i < strlen(name); i++) {
        if (!isprint(name[i]))
            return NULL;
    }
    assert(0);
    for (int i = 0; i < 40; i++)
        printf("a");
    printf("   ");
    fflush(stdout);
    printf("%s\n", name);
    return name;
}